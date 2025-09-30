if(NOT BUILD_CLIENT OR NOT BUILD_RENDERER_TERRAIN)
    return()
endif()

include(utils/set_output_dirs)

set(RENDERER_TERRAIN_SOURCES
    ${SOURCE_DIR}/renderer_terrain/tr_terrain_stub.c
    ${SOURCE_DIR}/renderer_terrain/tr_terrain_mesh.c
)

set(RENDERER_TERRAIN_BINARY renderer_terrain)

add_library(${RENDERER_TERRAIN_BINARY} SHARED ${RENDERER_TERRAIN_SOURCES})

set_output_dirs(${RENDERER_TERRAIN_BINARY} SUBDIRECTORY "my_diablo_output")
