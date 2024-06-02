CREATE_PROJECT(Kernel
	TYPE "STATIC"
	SOURCES
		"${PROJECT_SOURCE_DIR}/Source/Library"
		"${PROJECT_SOURCE_DIR}/Source/Platforms/${CMAKE_SYSTEM_NAME}"
	INCLUDE_DIRECTORIES
		"${PROJECT_SOURCE_DIR}/Source/Library"
		"${PROJECT_SOURCE_DIR}/Source/Platforms/${CMAKE_SYSTEM_NAME}"
	PRECOMPILED_HEADER
		"${PROJECT_SOURCE_DIR}/Source/Library/PrecompiledHeaders.h"
	SOURCE_GROUP
		"${PROJECT_SOURCE_DIR}/Source"
	LINK_LIBRARIES
		SDL2main
		SDL2-static
		imgui
		stb
		pugixml-static
		fmt
		gdi32
		opengl32
		imm32
		dwmapi
	COMPILE_OPTIONS
		/W4
		/WX
)

CREATE_PROJECT(Application
	TYPE "WIN32"
	SOURCES
		"${PROJECT_SOURCE_DIR}/Source/Main.cpp"
		"${PROJECT_SOURCE_DIR}/Resource.rc"
	SOURCE_GROUP
		"${PROJECT_SOURCE_DIR}"
	LINK_LIBRARIES
		Kernel
)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "Application")
set_target_properties("Application" PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})