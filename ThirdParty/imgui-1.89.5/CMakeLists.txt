project(imgui)

CREATE_PROJECT(${PROJECT_NAME}
	TYPE "STATIC"
	SOURCES
		"${PROJECT_SOURCE_DIR}/imgui.cpp"
		"${PROJECT_SOURCE_DIR}/imgui_demo.cpp"
		"${PROJECT_SOURCE_DIR}/imgui_draw.cpp"
		"${PROJECT_SOURCE_DIR}/imgui_tables.cpp"
		"${PROJECT_SOURCE_DIR}/imgui_widgets.cpp"
		"${PROJECT_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
		"${PROJECT_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp"
		"${PROJECT_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp"
	INCLUDE_DIRECTORIES
		"${PROJECT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/backends"
		"${PROJECT_SOURCE_DIR}/misc/freetype"
	SOURCE_GROUP
		"${PROJECT_SOURCE_DIR}"
	LINK_LIBRARIES
		glfw
		freetype
	COMPILE_DEFINITIONS
		IMGUI_ENABLE_FREETYPE
)