cmake_minimum_required(VERSION 3.16)

project(
	DiskUsage
	VERSION 0.1.0
	DESCRIPTION "Tool that uses a chart to display disk usage information."
	HOMEPAGE_URL "https://github.com/sergei-belov-personal/disk-usage"
	LANGUAGES CXX
)

# Create project from all source files located in PROJECT_SOURCE_PATH.
function(CreateProject PROJECT_NAME PROJECT_SOURCE_PATH PLATFORM)
	# Project header and source files.
	file(GLOB_RECURSE PROJECT_SOURCE_FILES ${PROJECT_SOURCE_PATH}/*.cc)
	file(GLOB_RECURSE PROJECT_HEADER_FILES ${PROJECT_SOURCE_PATH}/*.h)

	# Source files grouping for editor.
	source_group(TREE ${PROJECT_SOURCE_PATH} FILES ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILES})

	# Create project executable.
	add_executable(${PROJECT_NAME} ${PLATFORM} ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILES})

	# Include directories.
	target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_PATH})

	# Precompiled headers.
	target_precompile_headers(${PROJECT_NAME} PRIVATE ${PRECOMPILED_HEADERS_PATH})
endfunction()

# C++ standard and extensions.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configurations.
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Suppress regeneration target.
set(CMAKE_SUPPRESS_REGENERATION ON)

# Predefined targets folder.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER Build)

# Path to project source directory.
set(PROJECT_SOURCE_PATH ${PROJECT_SOURCE_DIR}/Source)

# Path to test projects directory.
set(TEST_PROJECTS_PATH ${PROJECT_SOURCE_DIR}/Tests)

# Path to precompiled headers file.
set(PRECOMPILED_HEADERS_PATH ${PROJECT_SOURCE_PATH}/PrecompiledHeaders.h)

# Path to Google Test library source directory.
set(GOOGLE_TEST_PATH ${PROJECT_SOURCE_DIR}/ThirdParty/googletest-1.13.0)

# Path to SDL library source directory.
set(SDL_PATH ${PROJECT_SOURCE_DIR}/ThirdParty/SDL-release-2.26.5)

# Create main project.
CreateProject(${PROJECT_NAME} ${PROJECT_SOURCE_PATH} "")

# Startup project and debugger directory.
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# Include Google Test.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

add_subdirectory(${GOOGLE_TEST_PATH})

set_target_properties(
	gmock
	gmock_main
	gtest
	gtest_main
	PROPERTIES FOLDER "Third Party"
)

# Include SDL.
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "" FORCE)

set(SDL_TEST OFF CACHE BOOL "" FORCE)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

add_subdirectory(${SDL_PATH})

set_target_properties(
	SDL2main
	SDL2-static
	sdl_headers_copy
	PROPERTIES FOLDER "Third Party"
)

target_link_libraries(${PROJECT_NAME}
	SDL2main
	SDL2-static
	gdi32
	opengl32
	imm32
)

target_include_directories(${PROJECT_NAME} PRIVATE ${SDL_PATH}/include)

# Include test projects.
file(GLOB TEST_PROJECTS RELATIVE ${TEST_PROJECTS_PATH} ${TEST_PROJECTS_PATH}/*)
foreach(TEST_PROJECT ${TEST_PROJECTS})
	set(TEST_PROJECT_SOURCE_PATH ${TEST_PROJECTS_PATH}/${TEST_PROJECT})

	# Any directories inside TEST_PROJECTS_PATH are test projects.
	if (NOT IS_DIRECTORY ${TEST_PROJECT_SOURCE_PATH})
		continue()
	endif()

	# Create test project.
	CreateProject(${TEST_PROJECT} ${TEST_PROJECT_SOURCE_PATH} "")

	target_link_libraries(${TEST_PROJECT} gtest)
	set_target_properties(${TEST_PROJECT} PROPERTIES FOLDER "Tests")
endforeach()