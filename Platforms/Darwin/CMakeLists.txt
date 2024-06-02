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
		"-framework OpenGL"
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