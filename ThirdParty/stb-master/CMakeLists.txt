project(stb)

CREATE_PROJECT(${PROJECT_NAME}
	TYPE "INTERFACE"
	SOURCES
		"${PROJECT_SOURCE_DIR}/stb_image.h"
	INCLUDE_DIRECTORIES
		"${PROJECT_SOURCE_DIR}"	
	SOURCE_GROUP
		"${PROJECT_SOURCE_DIR}"
)