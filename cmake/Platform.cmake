IF(WIN32)
	IF(MSVC)
		IF(CMAKE_GENERATOR MATCHES "Win64")
			SET(KLAYGE_ARCH_NAME "x64")
			SET(KLAYGE_VS_PLATFORM_NAME "x64")
		ELSEIF(CMAKE_GENERATOR MATCHES "ARM")
			SET(KLAYGE_ARCH_NAME "arm")
			SET(KLAYGE_VS_PLATFORM_NAME "ARM")
		ELSE()
			SET(KLAYGE_ARCH_NAME "x86")
			SET(KLAYGE_VS_PLATFORM_NAME "Win32")
		ENDIF()
	ENDIF()
	IF(WINDOWS_STORE)
		SET(KLAYGE_PLATFORM_NAME "win_store")
		SET(KLAYGE_PLATFORM_WINDOWS_STORE TRUE)
		SET(KLAYGE_PLATFORM_WINDOWS_RUNTIME TRUE)
	ELSEIF(WINDOWS_PHONE)
		SET(KLAYGE_PLATFORM_NAME "win_phone")
		SET(KLAYGE_PLATFORM_WINDOWS_PHONE TRUE)
		SET(KLAYGE_PLATFORM_WINDOWS_RUNTIME TRUE)
	ELSE()
		SET(KLAYGE_PLATFORM_NAME "win")
		SET(KLAYGE_PLATFORM_WINDOWS_DESKTOP TRUE)
	ENDIF()
	SET(KLAYGE_PLATFORM_WINDOWS TRUE)
ELSEIF(ANDROID)
	SET(KLAYGE_PLATFORM_NAME "android")
	SET(KLAYGE_PLATFORM_ANDROID TRUE)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	SET(KLAYGE_PLATFORM_NAME "linux")
	SET(KLAYGE_PLATFORM_LINUX TRUE)
ELSEIF(IOS)
	SET(KLAYGE_PLATFORM_NAME "ios")
	SET(KLAYGE_PLATFORM_IOS TRUE)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	SET(KLAYGE_PLATFORM_NAME "darwin")
	SET(KLAYGE_PLATFORM_DARWIN TRUE)
ENDIF()

IF(NOT KLAYGE_ARCH_NAME)
	IF((CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64") OR (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64"))
		SET(KLAYGE_ARCH_NAME "x64")
	ELSE()
		SET(KLAYGE_ARCH_NAME "x86")
	ENDIF()
ENDIF()

SET(KLAYGE_PLATFORM_NAME ${KLAYGE_PLATFORM_NAME}_${KLAYGE_ARCH_NAME})

IF(KLAYGE_PLATFORM_ANDROID OR KLAYGE_PLATFORM_IOS)
	SET(KLAYGE_PREFERRED_LIB_TYPE "STATIC")
ELSE()
	SET(KLAYGE_PREFERRED_LIB_TYPE "SHARED")
ENDIF()
