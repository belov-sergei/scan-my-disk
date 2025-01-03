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
		pugixml-static
		fmt
		utf8proc
		zlibstatic
		png_static
		freetype
		"-framework OpenGL"
	COMPILE_DEFINITIONS
		GL_SILENCE_DEPRECATION
	COMPILE_OPTIONS
		-Wall
		-Wextra
		-Wpedantic
		-Werror
)

CREATE_PROJECT(Application
	TYPE "MACOSX_BUNDLE"
	SOURCES
		"${PROJECT_SOURCE_DIR}/Source/Main.cpp"
	SOURCE_GROUP
		"${PROJECT_SOURCE_DIR}"
	LINK_LIBRARIES
		Kernel
)

set_target_properties("Application" PROPERTIES XCODE_GENERATE_SCHEME TRUE)
set_target_properties("Application" PROPERTIES XCODE_SCHEME_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})