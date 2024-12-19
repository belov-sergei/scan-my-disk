CREATE_PROJECT(Kernel
	TYPE "STATIC"
	SOURCES
		"${PROJECT_SOURCE_DIR}/Source/Library"
		"${PROJECT_SOURCE_DIR}/Source/Platforms/${CMAKE_SYSTEM_NAME}"
	INCLUDE_DIRECTORIES
		"${PROJECT_SOURCE_DIR}/Source/Library"
		"${PROJECT_SOURCE_DIR}/Source/Platforms/${CMAKE_SYSTEM_NAME}"
		"${PROJECT_SOURCE_DIR}/ThirdParty/tracy-0.11.1/public"
	PRECOMPILED_HEADER
		"${PROJECT_SOURCE_DIR}/Source/Library/PrecompiledHeaders.h"
	SOURCE_GROUP
		"${PROJECT_SOURCE_DIR}/Source"
	LINK_LIBRARIES
		imgui
		stb
		pugixml-static
		fmt
		gdi32
		opengl32
		imm32
		dwmapi
		glfw
		utf8proc
		zlibstatic
		png_static
	COMPILE_DEFINITIONS
		WIN32_LEAN_AND_MEAN
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
	WORKING_DIRECTORY
		"${PROJECT_SOURCE_DIR}"
	LINK_LIBRARIES
		Kernel
)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "Application")
