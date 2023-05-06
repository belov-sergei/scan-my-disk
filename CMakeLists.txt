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

# Create main project.
CreateProject(${PROJECT_NAME} ${PROJECT_SOURCE_PATH} "")

# Startup project and debugger directory.
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})