if( WIN32 AND MSVC )
	# Microsoft Speech SDK & Speech DLL
	find_path( SPEECHSDK_INCLUDE_DIR
		NAMES SpeechSDK.h sapi.h sapiddk.h sperror.h sphelper.h
		PATHS
			${CMAKE_CURRENT_SOURCE_DIR}/dependencies/speech/include
			$ENV{KINECTSPEECH_DIR}Include
			NO_SYSTEM_ENVIRONMENT_PATH
	)
	find_library( SPEECHSDK_LIBRARY
		NAMES SpeechSDK sapi
		PATHS
			${CMAKE_CURRENT_SOURCE_DIR}/dependencies/speech/lib
			$ENV{KINECTSPEECH_DIR}Lib
			NO_SYSTEM_ENVIRONMENT_PATH
		)
	find_path( SPEECHSDK_DLL
		NAMES SpeechSDK.dll
		PATHS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/speech/dist
	)
endif()

include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( SPEECHSDK REQUIRED_VARS SPEECHSDK_LIBRARY SPEECHSDK_INCLUDE_DIR SPEECHSDK_DLL )

if( SPEECHSDK_FOUND )
	set( SPEECHSDK_DRIVER_DIRS ${SPEECHSDK_DLL} )
	set( SPEECHSDK_LIBRARIES ${SPEECHSDK_LIBRARY} )
	set( SPEECHSDK_INCLUDE_DIRS ${SPEECHSDK_INCLUDE_DIR} )
endif()

mark_as_advanced( SPEECHSDK_DLL SPEECHSDK_LIBRARY SPEECHSDK_INCLUDE_DIR )
