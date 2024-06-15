# Creates a project with the specified settings.
function(CREATE_PROJECT PROJECT_NAME)
	cmake_parse_arguments(PROJECT
		""
		"TYPE;PRECOMPILED_HEADER;SOURCE_GROUP;FOLDER;WORKING_DIRECTORY"
		"SOURCES;EXCLUDE_PATTERNS;INCLUDE_DIRECTORIES;LINK_LIBRARIES;COMPILE_DEFINITIONS;COMPILE_OPTIONS"
		${ARGN}
	)

	if (NOT PROJECT_TYPE)
		add_executable(${PROJECT_NAME})
	elseif(PROJECT_TYPE STREQUAL "STATIC" OR PROJECT_TYPE STREQUAL "SHARED" OR PROJECT_TYPE STREQUAL "INTERFACE")
		add_library(${PROJECT_NAME} ${PROJECT_TYPE})
	else()
		add_executable(${PROJECT_NAME} ${PROJECT_TYPE})
	endif()

	foreach(PATH ${PROJECT_SOURCES})
		if (EXISTS ${PATH})
			if (IS_DIRECTORY ${PATH})
				file(GLOB_RECURSE FOUND_FILES
					"${PATH}/*.h"
					"${PATH}/*.mm"
					"${PATH}/*.c"
					"${PATH}/*.cpp"
				)
				list(APPEND FILE_LIST ${FOUND_FILES})
			else()
				list(APPEND FILE_LIST ${PATH})
			endif()
		endif()
	endforeach()

	foreach(PATTERN ${PROJECT_EXCLUDE_PATTERNS})
		list(FILTER FILE_LIST EXCLUDE REGEX ${PATTERN})
	endforeach()

	if (FILE_LIST)
		if(PROJECT_SOURCE_GROUP)
			source_group(TREE ${PROJECT_SOURCE_GROUP} FILES ${FILE_LIST})
		endif()
		
		target_sources(${PROJECT_NAME} PRIVATE ${FILE_LIST})
	endif()

	if (PROJECT_INCLUDE_DIRECTORIES)
		if (PROJECT_TYPE STREQUAL "INTERFACE")
        	set(INCLUDE_DIRECTORIES_KEYWORD INTERFACE)
		else()
			set(INCLUDE_DIRECTORIES_KEYWORD PUBLIC)
		endif()

		target_include_directories(${PROJECT_NAME} ${INCLUDE_DIRECTORIES_KEYWORD} ${PROJECT_INCLUDE_DIRECTORIES})
	endif()

	if (PROJECT_PRECOMPILED_HEADER)
		target_precompile_headers(${PROJECT_NAME} PUBLIC ${PROJECT_PRECOMPILED_HEADER})
	endif()

	if (PROJECT_LINK_LIBRARIES)
		target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBRARIES})
	endif()

	if (PROJECT_COMPILE_DEFINITIONS)
		target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_DEFINITIONS})
	endif()

	if (PROJECT_COMPILE_OPTIONS)
		target_compile_options(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_OPTIONS})
	endif()

	if (PROJECT_FOLDER)
		set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_FOLDER})
	endif()
	
	if (PROJECT_WORKING_DIRECTORY)
		set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_WORKING_DIRECTORY})
	endif()
endfunction()

# Includes and configures a Third Party project.
function(THIRD_PARTY_PROJECT DIRECTORY_PATH)
	cmake_parse_arguments(THIRD_PARTY
		""
		""
		"TARGETS;OPTIONS"
		${ARGN}
	)

	set(OPTION_NAME "")
	foreach(VALUE ${THIRD_PARTY_OPTIONS})
		if(OPTION_NAME STREQUAL "")
			set(OPTION_NAME ${VALUE})
			continue()
		endif()

		set(${OPTION_NAME} ${VALUE} CACHE INTERNAL "" FORCE)
		set(OPTION_NAME "")
	endforeach()

	add_subdirectory(${DIRECTORY_PATH})

	foreach(TARGET ${THIRD_PARTY_TARGETS})
		set_target_properties(${TARGET} PROPERTIES FOLDER "ThirdParty")
	endforeach()
endfunction()