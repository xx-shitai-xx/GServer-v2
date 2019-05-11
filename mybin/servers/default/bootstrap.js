'use strict';

(function (env) {
    print("Test Environment ", env);
    print("Test Environment Type: ", typeof env);
	
	/**
	 * Events -> onCreated(npc, args...)
	 */
    env.setCallBack("npc.created", function (npc, ...args) {
        try {
			// Set whether the npc supports these events
			env.setNpcEvents(npc,
				(npc.onCreated && 1 << 0) |
				(npc.onTimeout && 1 << 1) |
				(npc.onPlayerChats && 1 << 2) |
				(npc.onPlayerEnters && 1 << 3) |
				(npc.onPlayerLeaves && 1 << 4) |
				(npc.onPlayerTouchsMe && 1 << 5)
			);

            if (npc.onCreated)
                npc.onCreated.apply(npc, args);
        } catch (e) {
            env.reportException("NPC Exception at " + npc.levelname + "," + npc.x + "," + npc.y + ": " + e.name + " - " + e.message);
        }
    });

	/**
	 * Event -> onPlayerChats(npc, player, message)
	 */
    env.setCallBack("npc.playerchats", function (npc, player, message) {
        try {
            if (npc.onPlayerChats)
                npc.onPlayerChats(player, message);
        } catch (e) {
            env.reportException("NPC Exception at " + npc.levelname + "," + npc.x + "," + npc.y + ": " + e.name + " - " + e.message);
        }
    });
    
	/**
	 * Event -> onPlayerEnters(npc, player)
	 */
    env.setCallBack("npc.playerenters", function (npc, player) {
        try {
            if (npc.onPlayerEnters)
                npc.onPlayerEnters(player);
        } catch (e) {
            env.reportException("NPC Exception at " + npc.levelname + "," + npc.x + "," + npc.y + ": " + e.name + " - " + e.message);
        }
	});
	
	/**
	 * Event -> onPlayerLeaves(npc, player)
	 */
    env.setCallBack("npc.playerleaves", function (npc, player) {
        try {
            if (npc.onPlayerLeaves)
                npc.onPlayerLeaves(player);
        } catch (e) {
            env.reportException("NPC Exception at " + npc.levelname + "," + npc.x + "," + npc.y + ": " + e.name + " - " + e.message);
        }
	});
	
	
	/**
	 * Event -> onPlayerTouchsMe(npc, player)
	 */
    env.setCallBack("npc.playertouchsme", function (npc, player) {
        try {
            if (npc.onPlayerTouchsMe)
                npc.onPlayerTouchsMe(player);
        } catch (e) {
            env.reportException("NPC Exception at " + npc.levelname + "," + npc.x + "," + npc.y + ": " + e.name + " - " + e.message);
        }
	});

    /**
	 * Event -> onTimeout(npc, args...)
	 */
    env.setCallBack("npc.timeout", function (npc, ...args) {
        try {
            if (npc.onTimeout)
                npc.onTimeout.apply(npc, args);
        } catch (e) {
            env.reportException("NPC Timeout Exception at " + npc.levelname + "," + npc.x + "," + npc.y + ": " + e.name + " - " + e.message);
        }
    });

    /*
     * Event -> Triggeractions
     */
    env.setCallBack("npc.trigger", function (npc, func, data) {
        try {
            func.call(npc, data);
        } catch (e) {
            env.reportException("NPC Trigger Exception at " + npc.levelname + "," + npc.x + "," + npc.y + ": " + e.name + " - " + e.message);
        }
    });

    /**
	 * Events -> weapon.onCreated(npc, args...)
	 */
    env.setCallBack("weapon.created", function (weapon, ...args) {
        try {
            if (weapon.onCreated)
                weapon.onCreated.apply(weapon, args);
        } catch (e) {
            env.reportException("Weapon Exception at " + weapon.name + ": " + e.name + " - " + e.message);
        }
    });

    /*
     * Event -> weapon.onActionServerSide(player, data)
     */
    env.setCallBack("weapon.serverside", function (weapon, player, data) {
        try {
            if (weapon.onActionServerSide)
                weapon.onActionServerSide(player, data);
        } catch (e) {
            env.reportException("Weapon Trigger Exception at " + weapon.name + ": " + e.name + " - " + e.message);
        }
    });
});
