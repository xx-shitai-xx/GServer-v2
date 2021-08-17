#include "IDebug.h"

#include "TWeapon.h"
#include "TServer.h"
#include "TLevelItem.h"
#include "TNPC.h"
#include "IEnums.h"
#include "IUtil.h"

// GS2 Compiler includes
#include "GS2Context.h"

#ifdef V8NPCSERVER
#include "TPlayer.h"
#endif

// -- Constructor: Default Weapons -- //
TWeapon::TWeapon(TServer *pServer, LevelItemType pId)
: server(pServer), mModTime(0), mWeaponDefault(pId)
#ifdef V8NPCSERVER
, _scriptObject(0), _scriptExecutionContext(pServer->getScriptEngine())
#endif
{
	_weaponName = convertCString(TLevelItem::getItemName(mWeaponDefault));
}

// -- Constructor: Weapon Script -- //
TWeapon::TWeapon(TServer *pServer, std::string pName, std::string pImage, std::string pScript, const time_t pModTime, bool pSaveWeapon)
: server(pServer), _weaponName(std::move(pName)), mModTime(pModTime), mWeaponDefault(LevelItemType::INVALID)
#ifdef V8NPCSERVER
, _scriptObject(0), _scriptExecutionContext(pServer->getScriptEngine())
#endif
{
	// Update Weapon
	this->updateWeapon(std::move(pImage), std::move(pScript), pModTime, pSaveWeapon);
}

TWeapon::~TWeapon()
{
#ifdef V8NPCSERVER
	freeScriptResources();
#endif
}

// -- Function: Load Weapon -- //
TWeapon * TWeapon::loadWeapon(const CString& pWeapon, TServer *server)
{
	// File Path
	CString fileName = server->getServerPath() << "weapons" << CFileSystem::getPathSeparator() << pWeapon;

	// Load File
	CString fileData;
	if (!fileData.load(fileName))
		return nullptr;

	fileData.removeAllI("\r");

	// Grab some information.
	bool has_script = (fileData.find("SCRIPT") != -1 ? true : false);
	bool has_scriptend = (fileData.find("SCRIPTEND") != -1 ? true : false);
	bool found_scriptend = false;

	// Parse header
	CString headerLine = fileData.readString("\n");
	if (headerLine != "GRAWP001")
		return nullptr;

	// Definitions
	CString byteCodeFile, byteCodeData;
	std::string weaponImage, weaponName, weaponScript;

	// Parse File
	while (fileData.bytesLeft())
	{
		CString curLine = fileData.readString("\n");

		// Find Command
		CString curCommand = curLine.readString();

		// Parse Line
		if (curCommand == "REALNAME")
			weaponName = convertCString(curLine.readString(""));
		else if (curCommand == "IMAGE")
			weaponImage = convertCString(curLine.readString(""));
		else if (curCommand == "BYTECODE")
		{
			CString fileName = curLine.readString("");

			byteCodeData.load(server->getServerPath() << "weapon_bytecode/" << fileName);
			if (!byteCodeData.isEmpty())
				byteCodeFile = fileName;
		}
		else if (curCommand == "SCRIPT")
		{
			do {
				curLine = fileData.readString("\n");
				if (curLine == "SCRIPTEND")
				{
					found_scriptend = true;
					break;
				}

				weaponScript.append(curLine.text()).append("\n");
			} while (fileData.bytesLeft());
		}
	}

	// Valid Weapon Name?
	if (weaponName.empty())
		return nullptr;

	// Give a warning if our weapon was malformed.
	if (has_scriptend && !found_scriptend)
	{
		server->getServerLog().out("[%s] WARNING: Weapon %s is malformed.\n", server->getName().text(), weaponName.c_str());
		server->getServerLog().out("[%s] SCRIPTEND needs to be on its own line.\n", server->getName().text());
	}

	// Give a warning if both a script and a bytecode was found.
	if (!weaponScript.empty() && !byteCodeData.isEmpty())
		server->getServerLog().out("[%s] WARNING: Weapon %s includes both script and bytecode.  Using bytecode.\n", server->getName().text(), weaponName.c_str());
	
	auto weapon = new TWeapon(server, weaponName, weaponImage, weaponScript, 0);
	if (!byteCodeData.isEmpty() && weapon->_bytecode.isEmpty())
	{
		weapon->_bytecode = byteCodeData;
		weapon->_bytecodeFile = byteCodeFile;
	}

	return weapon;
}

// -- Function: Save Weapon -- //
bool TWeapon::saveWeapon()
{
	// Don't save default weapons / empty weapons
	if (this->isDefault() || _weaponName.empty())
		return false;

	// Prevent the loading/saving of filenames with illegal characters.
	CString name = _weaponName;
	name.replaceAllI("\\", "_");
	name.replaceAllI("/", "_");
	name.replaceAllI("*", "@");
	name.replaceAllI(":", ";");
	name.replaceAllI("?", "!");
	CString filename = server->getServerPath() << "weapons" << CFileSystem::getPathSeparator() << "weapon" << name << ".txt";

	// Write the File.
	CString output = "GRAWP001\r\n";
	output << "REALNAME " << _weaponName << "\r\n";
	output << "IMAGE " << _weaponImage << "\r\n";
	output << "BYTECODE " << "weapon" << _weaponName << ".gs2bc" << "\r\n";

	if (_source)
	{
		output << "SCRIPT\r\n";
		output << CString(_source.getSource()).replaceAll("\n", "\r\n");
		
		// Append a new line to the end of the script if one doesn't exist.
		if (_source.getSource().back() != '\n')
			output << "\r\n";

		output << "SCRIPTEND\r\n";
	}

	// Save bytecode
	CString byteCodeFileName = server->getServerPath() << "weapons_bytecode" << CFileSystem::getPathSeparator() << "weapon" << name << ".gs2bc";
	_bytecode.save(byteCodeFileName);

	// Save it.
	return output.save(filename);
}

// -- Function: Get Player Packet -- //
CString TWeapon::getWeaponPacket(bool forceGS1) const
{
	if (this->isDefault())
		return CString() >> (char)PLO_DEFAULTWEAPON >> (char)mWeaponDefault;

	if (_bytecode.isEmpty() || forceGS1)
	{
		return CString() >> (char)PLO_NPCWEAPONADD
			>> (char)_weaponName.length() << _weaponName
			>> (char)NPCPROP_IMAGE >> (char)_weaponImage.length() << _weaponImage
			>> (char)NPCPROP_SCRIPT >> (short)clientScriptFormatted.length() << clientScriptFormatted;
	}
	else
	{
		CString out;
		out >> (char)PLO_NPCWEAPONADD >> (char)_weaponName.length() << _weaponName
			>> (char)NPCPROP_IMAGE >> (char)_weaponImage.length() << _weaponImage
			>> (char)NPCPROP_CLASS >> (short)0 << "\n";

		CString b = _bytecode;

		unsigned char id = b.readGUChar();
		CString header = b.readChars(b.readGUShort());
		CString header2 = header.guntokenize();

		CString type = header2.readString("\n");
		CString name = header2.readString("\n");
		CString unknown = header2.readString("\n");
		CString hash = header2.readString("\n");

		// Get the mod time and send packet 197.
		CString smod = CString() >> (long long)time(0);
		smod.gtokenizeI();
		out >> (char)PLO_UNKNOWN197 << header << "," << smod << "\n";

		// Add to the output stream.
		out >> (char)PLO_RAWDATA >> (int)b.length() << "\n";
		out << b;

		/*
		for (std::vector<std::pair<CString, CString> >::const_iterator i = mByteCode.begin(); i != mByteCode.end(); ++i)
		{
			CString b = i->second;

			unsigned char id = b.readGUChar();
			CString header = b.readChars(b.readGUShort());
			CString header2 = header.guntokenize();

			CString type = header2.readString("\n");
			CString name = header2.readString("\n");
			CString unknown = header2.readString("\n");
			CString hash = header2.readString("\n");

			// Get the mod time and send packet 197.
			CString smod = CString() >> (long long)time(0);
			smod.gtokenizeI();
			out >> (char)PLO_UNKNOWN197 << header << "," << smod << "\n";

			// Add to the output stream.
			out >> (char)PLO_RAWDATA >> (int)b.length() << "\n";
			out << b;
		}
		*/

		return out;
	}
}

// -- Function: Update Weapon Image/Script -- //
void TWeapon::updateWeapon(std::string pImage, std::string pCode, const time_t pModTime, bool pSaveWeapon)
{
#ifdef V8NPCSERVER
	// Clear script function
	if (_source || _scriptExecutionContext.hasActions())
		freeScriptResources();
#endif

	_source = SourceCode{ std::move(pCode) };
	_weaponImage = std::move(pImage);
	setModTime(pModTime == 0 ? time(0) : pModTime);

#ifdef V8NPCSERVER
	// Compile and execute the script.
	CScriptEngine *scriptEngine = server->getScriptEngine();
	bool executed = scriptEngine->ExecuteWeapon(this);
	if (executed)
	{
		SCRIPTENV_D("WEAPON SCRIPT COMPILED\n");

		if (!_source.getServerSide().empty()) {
			_scriptExecutionContext.addAction(scriptEngine->CreateAction("weapon.created", _scriptObject));
			scriptEngine->RegisterWeaponUpdate(this);
		}
	}
	else
		SCRIPTENV_D("Could not compile weapon script\n");
#endif

	// Compile GS2 code
	auto gs2Script = _source.getClientGS2();
	if (!gs2Script.empty())
	{
		GS2Context context;
		auto byteCode = context.compile(std::string{ gs2Script }, "weapon", _weaponName, true);

		if (!context.hasErrors())
		{
			_bytecode.clear();
			_bytecode.writeGChar(PLO_NPCWEAPONSCRIPT);
			_bytecode.write((const char*)byteCode.buffer(), byteCode.length());
		}
		else
		{
			printf("Compilation Error: %s\n", context.getErrors()[0].msg().c_str());
		}
	}
	
	// Save Weapon
	if (pSaveWeapon)
		saveWeapon();
}

void TWeapon::setClientScript(const CString& pScript)
{
	// Remove any comments in the code
	CString formattedScript = removeComments(pScript);
	clientScriptFormatted.clear(formattedScript.length());

	// Split code into tokens, trim each line, and use the clientside line ending '\xa7'
	std::vector<CString> code = formattedScript.tokenize("\n");
	for (auto & it : code)
		clientScriptFormatted << it.trim() << "\xa7";
}

#ifdef V8NPCSERVER

void TWeapon::freeScriptResources()
{
	CScriptEngine *scriptEngine = server->getScriptEngine();

	scriptEngine->ClearCache<TWeapon>(_source.getServerSide());

	// Clear any queued actions
	if (_scriptExecutionContext.hasActions())
	{
		// Unregister npc from any queued event calls
		scriptEngine->UnregisterWeaponUpdate(this);

		// Reset execution
		_scriptExecutionContext.resetExecution();
	}

	// Delete script object
	if (_scriptObject)
	{
		delete _scriptObject;
		_scriptObject = nullptr;
	}
}

void TWeapon::queueWeaponAction(TPlayer *player, const std::string& args)
{
	CScriptEngine *scriptEngine = server->getScriptEngine();

	ScriptAction scriptAction = scriptEngine->CreateAction("weapon.serverside", _scriptObject, player->getScriptObject(), args);
	_scriptExecutionContext.addAction(scriptAction);
	scriptEngine->RegisterWeaponUpdate(this);
}

#endif