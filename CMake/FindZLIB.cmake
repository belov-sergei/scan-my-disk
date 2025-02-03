# Copyright ❤️ 2023-2025, Sergei Belov

# A workaround for building PNG and FreeType libraries. They try to find an already built one ZLIB,
# but in this project, ZLIB is built together with them, and at the time of project generation, it doesn't exist yet.

set(ZLIB_FOUND TRUE)
set(ZLIB_VERSION 1.3.1)

set(ZLIB_LIBRARIES
	zlibstatic
)

set(ZLIB_INCLUDE_DIRS
	"${CMAKE_SOURCE_DIR}/ThirdParty/zlib-1.3.1"
	"${CMAKE_BINARY_DIR}/ThirdParty/zlib-1.3.1"
)

add_library(ZLIB::ZLIB ALIAS ${ZLIB_LIBRARIES})