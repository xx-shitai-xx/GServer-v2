# Courtesy of: https://raw.githubusercontent.com/gwaldron/osgearth/master/CMakeModules/FindV8.cmake
#
# Locate V8
# This module defines
# V8_LIBRARY
# V8_FOUND, if false, do not try to link to V8
# V8_INCLUDE_DIR, where to find the headers

message("Looking for monolithic v8...")

if(NOSTATIC)
	message("Ignoring NOSTATIC for v8 lookup")
endif()

IF (NOT $ENV{V8_DIR} STREQUAL "")
	SET(V8_DIR $ENV{V8_DIR})
ENDIF()


SET(V8_LIBRARY_SEARCH_PATHS
	${V8_DIR}/
	${V8_DIR}/lib/
	${V8_DIR}/build/Release/lib/
	${V8_DIR}/build/Release/lib/third_party/icu/
	${V8_DIR}/build/Release/obj/
	${V8_DIR}/build/Release/obj/third_party/icu/
	${V8_DIR}/out/ia32.release/lib.target/
	${V8_DIR}/out/ia32.release/lib.target/third_party/icu/
	${V8_DIR}/out/ia32.release/obj/
	${V8_DIR}/out/ia32.release/obj/third_party/icu/
	${V8_DIR}/out.gn/ia32.release/lib.target/
	${V8_DIR}/out.gn/ia32.release/lib.target/third_party/icu/
	${V8_DIR}/out.gn/ia32.release/obj/
	${V8_DIR}/out.gn/ia32.release/obj/third_party/icu/
	${V8_DIR}/out/x64.release/lib.target/
	${V8_DIR}/out/x64.release/lib.target/third_party/icu/
	${V8_DIR}/out/x64.release/obj/
	${V8_DIR}/out/x64.release/obj/third_party/icu/
	${V8_DIR}/out.gn/x64.release/lib.target/
	${V8_DIR}/out.gn/x64.release/lib.target/third_party/icu/
	${V8_DIR}/out.gn/x64.release/obj/
	${V8_DIR}/out.gn/x64.release/obj/third_party/icu/
	${V8_DIR}/out.gn/x64.release.sample/lib.target/
	${V8_DIR}/out.gn/x64.release.sample/lib.target/third_party/icu/
	${V8_DIR}/out.gn/x64.release.sample/obj/
	${V8_DIR}/out.gn/x64.release.sample/obj/third_party/icu/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/x64.release/lib.target/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/x64.release/lib.target/third_party/icu/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/x64.release/obj/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/x64.release/obj/third_party/icu/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/x64.release.sample/lib.target/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/x64.release.sample/lib.target/third_party/icu/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/x64.release.sample/obj/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/x64.release.sample/obj/third_party/icu/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/arm.release/lib.target/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/arm.release/lib.target/third_party/icu/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/arm.release/obj/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/arm.release/obj/third_party/icu/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/arm.release.sample/lib.target/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/arm.release.sample/lib.target/third_party/icu/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/arm.release.sample/obj/
	${PROJECT_SOURCE_DIR}/dependencies/v8/out.gn/arm.release.sample/obj/third_party/icu/
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/lib
	/usr/lib
	/sw/lib
	/opt/local/lib
	/opt/csw/lib
	/opt/lib
	/usr/freeware/lib64

)

FIND_PATH(V8_INCLUDE_DIR v8.h
	${V8_DIR}
	${V8_DIR}/include
	${PROJECT_SOURCE_DIR}/dependencies/v8
	${PROJECT_SOURCE_DIR}/dependencies/v8/include
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/include
	/usr/include
	/sw/include # Fink
	/opt/local/include # DarwinPorts
	/opt/csw/include # Blastwave
	/opt/include
	/usr/freeware/include
	/devel
)

FIND_LIBRARY(V8_LIBRARY
	NAMES libv8_monolith.a v8_monolith.a v8_monolith libv8_monolith
	PATHS ${V8_LIBRARY_SEARCH_PATHS}
)

if(NOT V8_LIBRARY OR NOT V8_INCLUDE_DIR)
	if(NOT V8_LIBRARY)
		message("Couldn't find v8 library.")
	endif()
	if(NOT V8_INCLUDE_DIR)
		message("Couldn't find v8 include dir.")
	endif()
	message("Monolith search failed, looking for nuget package")

	find_path(V8_INCLUDE_DIR v8.h
		PATHS
		${PROJECT_SOURCE_DIR}/packages/v8-v142-x64.7.4.288.26/include
		${V8_DIR}/include)

	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		message("Searching for Release library as chosen")
		set(V8_LIBRARY_SEARCH_PATHS
			${PROJECT_SOURCE_DIR}/packages/v8-v142-x64.7.4.288.26/lib/Release
			${V8_DIR}/Release
			${V8_DIR}/lib/Release
		)
	else()
		if(NOT CMAKE_BUILD_TYPE)
			message("Searching for Debug library by default")
		elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
			message("Searching for Debug library as chosen")
		else()
			message("Build type not recognized, searching for Debug library")
		endif()
		set(V8_LIBRARY_SEARCH_PATHS
			${PROJECT_SOURCE_DIR}/packages/v8-v142-x64.7.4.288.26/lib/Debug
			${V8_DIR}/Debug
			${V8_DIR}/lib/Debug
		)
	endif()

	find_library(V8_LIBRARY v8.dll.lib
		PATHS ${V8_LIBRARY_SEARCH_PATHS}
	)

	if(NOT V8_LIBRARY)
		message("Couldn't find v8 library as nuget package")
	endif()
	if(NOT V8_INCLUDE_DIR)
		message("Couldn't find v8 include dir as nuget package")
	endif()

endif()
	
IF (V8_LIBRARY AND V8_INCLUDE_DIR)
	message("V8 found!")
	set(V8_FOUND TRUE)
ELSE()
	message("V8 lookup failed.")
ENDIF()

