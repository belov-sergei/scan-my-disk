cmake_minimum_required(VERSION 3.16)
include("CMakeCommon.cmake")

set(PROJECT_LANGUAGES CXX)

if (APPLE)
	list(APPEND PROJECT_LANGUAGES OBJCXX)
endif()

project(
	"Scan My Disk"
	VERSION 1.3
	DESCRIPTION "Interactive pie chart visualization of disk space usage."
	HOMEPAGE_URL "https://github.com/belov-sergei/scan-my-disk"
	LANGUAGES ${PROJECT_LANGUAGES}
)

# Language standard and extensions.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

# Build configurations.
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Disables ZERO_CHECK in the generated project.
set(CMAKE_SUPPRESS_REGENERATION TRUE)

# Use filters to organize build targets.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Folder name for ALL_BUILD, ZERO_CHECK, etc. targets.
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Build")

add_subdirectory("Platforms/${CMAKE_SYSTEM_NAME}")
add_subdirectory("ThirdParty")
add_subdirectory("Tools")

enable_testing()
add_subdirectory("Tests")