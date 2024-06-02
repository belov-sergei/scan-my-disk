project(imgui)

CREATE_PROJECT(${PROJECT_NAME}
	TYPE "STATIC"
	SOURCES
		"${PROJECT_SOURCE_DIR}/imgui.cpp"
		"${PROJECT_SOURCE_DIR}/imgui_demo.cpp"
		"${PROJECT_SOURCE_DIR}/imgui_draw.cpp"
		"${PROJECT_SOURCE_DIR}/imgui_tables.cpp"
		"${PROJECT_SOURCE_DIR}/imgui_widgets.cpp"
		"${PROJECT_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp"
		"${PROJECT_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
		"${PROJECT_SOURCE_DIR}/imgui.h"
		"${PROJECT_SOURCE_DIR}/backends/imgui_impl_sdl2.h"
		"${PROJECT_SOURCE_DIR}/backends/imgui_impl_opengl3.h"
	INCLUDE_DIRECTORIES
		"${PROJECT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/backends"
	SOURCE_GROUP
		"${PROJECT_SOURCE_DIR}"
	LINK_LIBRARIES
		SDL2-static
)