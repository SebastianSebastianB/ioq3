# my_diablo_client.cmake
# Build configuration for ioquake3_diablo.exe with modified server code from my_diablo_server/

if(NOT BUILD_CLIENT)
    return()
endif()

include(utils/add_git_dependency)
include(utils/set_output_dirs)
include(shared_sources)

include(renderer_common)

# Use modified server sources from my_diablo_server folder
set(MY_DIABLO_SERVER_SOURCES
    ${SOURCE_DIR}/my_diablo_server/sv_bot.c
    ${SOURCE_DIR}/my_diablo_server/sv_client.c
    ${SOURCE_DIR}/my_diablo_server/sv_ccmds.c
    ${SOURCE_DIR}/my_diablo_server/sv_game.c
    ${SOURCE_DIR}/my_diablo_server/sv_init.c
    ${SOURCE_DIR}/my_diablo_server/sv_main.c
    ${SOURCE_DIR}/my_diablo_server/sv_net_chan.c
    ${SOURCE_DIR}/my_diablo_server/sv_snapshot.c
    ${SOURCE_DIR}/my_diablo_server/sv_world.c
    # NOTE: sv_rankings.c removed - requires external ranking library
)

# Use modified client sources from my_diablo_client folder
set(MY_DIABLO_CLIENT_SOURCES
    ${SOURCE_DIR}/my_diablo_client/cl_cgame.c
    ${SOURCE_DIR}/my_diablo_client/cl_cin.c
    ${SOURCE_DIR}/my_diablo_client/cl_console.c
    ${SOURCE_DIR}/my_diablo_client/cl_input.c
    ${SOURCE_DIR}/my_diablo_client/cl_keys.c
    ${SOURCE_DIR}/my_diablo_client/cl_main.c
    ${SOURCE_DIR}/my_diablo_client/cl_net_chan.c
    ${SOURCE_DIR}/my_diablo_client/cl_parse.c
    ${SOURCE_DIR}/my_diablo_client/cl_scrn.c
    ${SOURCE_DIR}/my_diablo_client/cl_ui.c
    ${SOURCE_DIR}/my_diablo_client/cl_avi.c
    # NOTE: libmumblelink.c removed to avoid duplicate symbol errors
    ${SOURCE_DIR}/my_diablo_client/snd_altivec.c
    ${SOURCE_DIR}/my_diablo_client/snd_adpcm.c
    ${SOURCE_DIR}/my_diablo_client/snd_dma.c
    ${SOURCE_DIR}/my_diablo_client/snd_mem.c
    ${SOURCE_DIR}/my_diablo_client/snd_mix.c
    ${SOURCE_DIR}/my_diablo_client/snd_wavelet.c
    ${SOURCE_DIR}/my_diablo_client/snd_main.c
    ${SOURCE_DIR}/my_diablo_client/snd_codec.c
    ${SOURCE_DIR}/my_diablo_client/snd_codec_wav.c
    ${SOURCE_DIR}/my_diablo_client/snd_codec_ogg.c
    ${SOURCE_DIR}/my_diablo_client/snd_codec_opus.c
    ${SOURCE_DIR}/my_diablo_client/qal.c
    ${SOURCE_DIR}/my_diablo_client/snd_openal.c
    ${SOURCE_DIR}/sdl/sdl_input.c
    ${SOURCE_DIR}/sdl/sdl_snd.c
    ${CLIENT_PLATFORM_SOURCES}
)

add_git_dependency(${SOURCE_DIR}/my_diablo_client/cl_console.c)

# Use custom binary name for Diablo mod client
set(MY_DIABLO_CLIENT_BINARY "ioquake3_diablo")

# Use same definitions as regular client (CLIENT_DEFINITIONS is set by platform/library configs)

# Build binary with MY_DIABLO_SERVER_SOURCES and MY_DIABLO_CLIENT_SOURCES
list(APPEND MY_DIABLO_CLIENT_BINARY_SOURCES
    ${MY_DIABLO_SERVER_SOURCES}
    ${MY_DIABLO_CLIENT_SOURCES}
    ${COMMON_SOURCES}
    ${BOTLIB_SOURCES}
    ${SYSTEM_SOURCES}
    ${ASM_SOURCES}
    ${CLIENT_ASM_SOURCES}
    ${CLIENT_LIBRARY_SOURCES})

add_executable(${MY_DIABLO_CLIENT_BINARY} ${CLIENT_EXECUTABLE_OPTIONS} ${MY_DIABLO_CLIENT_BINARY_SOURCES})

target_include_directories(     ${MY_DIABLO_CLIENT_BINARY} PRIVATE ${CLIENT_INCLUDE_DIRS})
target_compile_definitions(     ${MY_DIABLO_CLIENT_BINARY} PRIVATE ${CLIENT_DEFINITIONS})
target_compile_options(         ${MY_DIABLO_CLIENT_BINARY} PRIVATE ${CLIENT_COMPILE_OPTIONS})
target_link_libraries(          ${MY_DIABLO_CLIENT_BINARY} PRIVATE ${COMMON_LIBRARIES} ${CLIENT_LIBRARIES})
target_link_options(            ${MY_DIABLO_CLIENT_BINARY} PRIVATE ${CLIENT_LINK_OPTIONS})

# Output to my_diablo_output subdirectory
set_output_dirs(${MY_DIABLO_CLIENT_BINARY} SUBDIRECTORY "my_diablo_output")

if(NOT USE_RENDERER_DLOPEN)
    target_sources(${MY_DIABLO_CLIENT_BINARY} PRIVATE
        # These are never simultaneously populated
        ${RENDERER_GL1_BINARY_SOURCES}
        ${RENDERER_GL2_BINARY_SOURCES})

    target_include_directories( ${MY_DIABLO_CLIENT_BINARY} PRIVATE ${RENDERER_INCLUDE_DIRS})
    target_compile_definitions( ${MY_DIABLO_CLIENT_BINARY} PRIVATE ${RENDERER_DEFINITIONS})
    target_compile_options(     ${MY_DIABLO_CLIENT_BINARY} PRIVATE ${RENDERER_COMPILE_OPTIONS})
    target_link_libraries(      ${MY_DIABLO_CLIENT_BINARY} PRIVATE ${RENDERER_LIBRARIES})
endif()

foreach(LIBRARY IN LISTS CLIENT_DEPLOY_LIBRARIES)
    add_custom_command(TARGET ${MY_DIABLO_CLIENT_BINARY} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${LIBRARY}
            $<TARGET_FILE_DIR:${MY_DIABLO_CLIENT_BINARY}>)

    install(FILES ${LIBRARY} DESTINATION
        # install() requires a relative path hence:
        $<PATH:RELATIVE_PATH,$<TARGET_FILE_DIR:${MY_DIABLO_CLIENT_BINARY}>,${CMAKE_BINARY_DIR}/$<CONFIG>>)
endforeach()

if(POST_CLIENT_CONFIGURE_FUNCTION)
    cmake_language(CALL ${POST_CLIENT_CONFIGURE_FUNCTION})
endif()
