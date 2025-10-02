# Custom Diablo Mod Renderer (based on OpenGL1 with increased poly limits)

if(NOT BUILD_CLIENT)
    return()
endif()

include(utils/set_output_dirs)
include(renderer_common)

set(RENDERER_MY_DIABLO_SOURCES
    ${SOURCE_DIR}/renderer_my_diablo/tr_altivec.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_animation.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_backend.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_bsp.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_cmds.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_curve.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_flares.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_image.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_init.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_light.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_main.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_marks.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_mesh.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_model.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_model_iqm.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_scene.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_shade.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_shade_calc.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_shader.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_shadows.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_sky.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_surface.c
    ${SOURCE_DIR}/renderer_my_diablo/tr_world.c
)

set(RENDERER_MY_DIABLO_BASENAME renderer_my_diablo)
set(RENDERER_MY_DIABLO_BINARY ${RENDERER_MY_DIABLO_BASENAME})

list(APPEND RENDERER_MY_DIABLO_BINARY_SOURCES
    ${RENDERER_COMMON_SOURCES}
    ${RENDERER_MY_DIABLO_SOURCES}
    ${SDL_RENDERER_SOURCES}
    ${RENDERER_LIBRARY_SOURCES})

list(APPEND RENDERER_MY_DIABLO_BINARY_SOURCES ${DYNAMIC_RENDERER_SOURCES})

add_library(${RENDERER_MY_DIABLO_BINARY} SHARED ${RENDERER_MY_DIABLO_BINARY_SOURCES})

target_link_libraries(      ${RENDERER_MY_DIABLO_BINARY} PRIVATE ${RENDERER_LIBRARIES})
target_include_directories( ${RENDERER_MY_DIABLO_BINARY} PRIVATE ${RENDERER_INCLUDE_DIRS})
target_compile_definitions( ${RENDERER_MY_DIABLO_BINARY} PRIVATE ${RENDERER_DEFINITIONS})
target_compile_options(     ${RENDERER_MY_DIABLO_BINARY} PRIVATE ${RENDERER_COMPILE_OPTIONS})
target_link_options(        ${RENDERER_MY_DIABLO_BINARY} PRIVATE ${RENDERER_LINK_OPTIONS})

set_output_dirs(${RENDERER_MY_DIABLO_BINARY})

# Set output to my_diablo_output directory
set_target_properties(${RENDERER_MY_DIABLO_BINARY} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/my_diablo_output"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/my_diablo_output"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/my_diablo_output"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/my_diablo_output"
)
