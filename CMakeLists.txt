cmake_minimum_required(VERSION 3.16)

# Check for supported generators (Xcode and Visual Studio).
if(NOT CMAKE_GENERATOR MATCHES "Xcode|Visual Studio")
	message(WARNING "This project has only been tested with Xcode and Visual Studio. Use at your own risk.")
endif()

project(
	ScanMyDisk
	VERSION 1.0
	DESCRIPTION "Interactive pie chart visualization of disk space usage."
	HOMEPAGE_URL "https://github.com/belov-sergei/scan-my-disk"
	LANGUAGES CXX
)

# Set C++ standard and disable extensions.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

# Set the possible configuration types.
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Build configurations" FORCE)

# Suppresses the ZERO_CHECK target in the generated project files.
set(CMAKE_SUPPRESS_REGENERATION TRUE)

# Enable the use of solution folders in IDEs like Visual Studio.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set the folder name for predefined targets such as ALL_BUILD, ZERO_CHECK, etc.
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Build")

# Check the platform and add global compile definitions.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_compile_definitions("WINDOWS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	add_compile_definitions("MACOS")
endif()

# Set the MSVC runtime library globally.
if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Define key paths for the project structure.
set(PROJECT_SOURCE_PATH "${PROJECT_SOURCE_DIR}/Source")
set(TEST_PROJECTS_PATH "${PROJECT_SOURCE_DIR}/Tests")
set(PRECOMPILED_HEADERS_PATH "${PROJECT_SOURCE_PATH}/PrecompiledHeaders.h")

# Set the base path for all third-party libraries.
set(THIRD_PARTY_PATH "${PROJECT_SOURCE_DIR}/ThirdParty")

# Define paths for third-party libraries.
set(GOOGLE_TEST_PATH "${THIRD_PARTY_PATH}/googletest-1.13.0")
set(SDL_PATH "${THIRD_PARTY_PATH}/SDL-release-2.26.5")
set(IMGUI_PATH "${THIRD_PARTY_PATH}/imgui-1.89.5")
set(STB_PATH "${THIRD_PARTY_PATH}/stb-master")
set(PUGIXML_PATH "${THIRD_PARTY_PATH}/pugixml-1.14")
set(FMT_PATH "${THIRD_PARTY_PATH}/fmt-10.1.1")

# Function to add source files to a target.
function (my_add_source TARGET_NAME TARGET_SOURCE_PATH)
	file(GLOB_RECURSE TARGET_SOURCE_FILES "${TARGET_SOURCE_PATH}/*.cpp" "${TARGET_SOURCE_PATH}/*.h")
	source_group(TREE ${TARGET_SOURCE_PATH} FILES ${TARGET_SOURCE_FILES})

	# Add the collected source files to the target.
	target_sources(${TARGET_NAME} PRIVATE ${TARGET_SOURCE_FILES})

	# Set include directories and precompiled headers.
	target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_PATH})
	target_precompile_headers(${TARGET_NAME} PRIVATE ${PRECOMPILED_HEADERS_PATH})

	# FIXME: Temporary dependency for tests until the main code is moved to a library.
	target_link_libraries(${TARGET_NAME} fmt)
endfunction()

# Create the project executable.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_executable(${PROJECT_NAME} WIN32 "Resource.rc")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	add_executable(${PROJECT_NAME} MACOSX_BUNDLE)
else()
	add_executable(${PROJECT_NAME})
endif()

my_add_source(${PROJECT_NAME} ${PROJECT_SOURCE_PATH})

# Configure startup project and debugger working directory.
if(CMAKE_GENERATOR MATCHES "Visual Studio")
	set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

if(CMAKE_GENERATOR MATCHES "Xcode")
	set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_GENERATE_SCHEME TRUE)
	set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_SCHEME_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

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
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(${PROJECT_NAME}
		gdi32
		opengl32
		imm32
		dwmapi
	)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	target_link_libraries(${PROJECT_NAME}
	 	"-framework OpenGL"
	)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${SDL_PATH}/include)

# Include IMGUI.
set(IMGUI_SOURCE_FILES
	${IMGUI_PATH}/imgui.cpp
	${IMGUI_PATH}/imgui_demo.cpp
	${IMGUI_PATH}/imgui_draw.cpp
	${IMGUI_PATH}/imgui_tables.cpp
	${IMGUI_PATH}/imgui_widgets.cpp
	${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp
	${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
)

set(IMGUI_HEADER_FILES
	${IMGUI_PATH}/imgui.h
	${IMGUI_PATH}/backends/imgui_impl_sdl2.h
	${IMGUI_PATH}/backends/imgui_impl_opengl3.h
)

add_library(imgui STATIC ${IMGUI_SOURCE_FILES} ${IMGUI_HEADER_FILES})

target_include_directories(imgui PRIVATE
	${IMGUI_PATH}
	${IMGUI_PATH}/backends
	
	${SDL_PATH}/include
)

set_target_properties(
	imgui
	PROPERTIES FOLDER "Third Party"
)

target_link_libraries(${PROJECT_NAME}
	imgui
)

target_include_directories(${PROJECT_NAME} PRIVATE
	${IMGUI_PATH}
	${IMGUI_PATH}/backends
)

# Include STB.
target_include_directories(${PROJECT_NAME} PRIVATE ${STB_PATH})

# Include Pugixml.
add_subdirectory(${PUGIXML_PATH})

set_target_properties(
	pugixml-static
	PROPERTIES FOLDER "Third Party"
)

target_link_libraries(${PROJECT_NAME}
	pugixml-static
)

# Include FMT.
add_subdirectory(${FMT_PATH})

set_target_properties(
	fmt
	PROPERTIES FOLDER "Third Party"
)

# Adds existing test projects. To create a new test project, make a copy of the Tests/Sample project.
# Each test project is set up to include the main project's include directory and is linked against the fmt library.
file(GLOB TEST_PROJECTS RELATIVE ${TEST_PROJECTS_PATH} ${TEST_PROJECTS_PATH}/*)
foreach(TEST_PROJECT ${TEST_PROJECTS})
	set(TEST_PROJECT_SOURCE_PATH ${TEST_PROJECTS_PATH}/${TEST_PROJECT})

	# Adds any directory inside TEST_PROJECTS_PATH as a test project.
	if(NOT IS_DIRECTORY ${TEST_PROJECT_SOURCE_PATH})
		continue()
	endif()

	add_executable(${TEST_PROJECT})
	my_add_source(${TEST_PROJECT} ${TEST_PROJECT_SOURCE_PATH})

	# Links the test project with the Google Test framework.
	target_link_libraries(${TEST_PROJECT} gtest)

	# Organizes the test projects into a "Tests" folder in supported IDEs.
	set_target_properties(${TEST_PROJECT} PROPERTIES FOLDER "Tests")
endforeach()