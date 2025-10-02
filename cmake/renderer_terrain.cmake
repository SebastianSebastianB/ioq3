if(NOT BUILD_CLIENT OR NOT BUILD_RENDERER_TERRAIN)
    return()
endif()

include(utils/set_output_dirs)

set(RENDERER_TERRAIN_SOURCES
    ${SOURCE_DIR}/renderer_terrain/tr_terrain_stub.c
    ${SOURCE_DIR}/renderer_terrain/tr_terrain_mesh.c
    ${SOURCE_DIR}/renderer_terrain/rt_impl.c
)

set(RENDERER_TERRAIN_BINARY renderer_terrain)

# SHARED library for dynamic loading by game modules (cgame, qagame, ui)
add_library(${RENDERER_TERRAIN_BINARY} SHARED ${RENDERER_TERRAIN_SOURCES})
target_compile_definitions(${RENDERER_TERRAIN_BINARY} PRIVATE RT_BUILDING_DLL)
set_output_dirs(${RENDERER_TERRAIN_BINARY} SUBDIRECTORY "my_diablo_output")

# STATIC library for linking into server executable (ioquake3_diablo.exe)
add_library(renderer_terrain_static STATIC ${RENDERER_TERRAIN_SOURCES})
target_compile_definitions(renderer_terrain_static PUBLIC RT_STATIC)
