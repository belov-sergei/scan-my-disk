# A workaround for building PNG and FreeType libraries. They try to find an already built one ZLIB,
# but in this project, ZLIB is built together with them, and at the time of project generation, it doesn't exist yet.

set(PNG_FOUND TRUE)
set(PNG_VERSION_STRING "1.6.44")

set(PNG_LIBRARIES
	png_static
)

set(PNG_INCLUDE_DIRS
	"${CMAKE_SOURCE_DIR}/ThirdParty/libpng-1.6.44"
)

add_library(PNG::PNG ALIAS ${PNG_LIBRARIES})