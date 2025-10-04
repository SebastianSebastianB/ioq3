# cmake/my_diablo_mod.cmake
message(STATUS "--- Defining build targets for My Diablo Mod ---")

include(utils/set_output_dirs)

set(DIABLO_SHARED_MATH
    "${SOURCE_DIR}/qcommon/q_math.c"
)

set(DIABLO_BG_SOURCES
    "${SOURCE_DIR}/my_diablo_game/bg_lib.c"
    "${SOURCE_DIR}/my_diablo_game/bg_misc.c"
    "${SOURCE_DIR}/my_diablo_game/bg_pmove.c"
    "${SOURCE_DIR}/my_diablo_game/bg_slidemove.c"
)

# --- HEIGHTMAP SYSTEM ---
set(DIABLO_HEIGHTMAP_SOURCES
    "${SOURCE_DIR}/my_diablo_heightmap/cm_heightmap.c"
    "${SOURCE_DIR}/my_diablo_heightmap/hm_loader.c"
)

# --- QAGAME ---
file(GLOB QAGAME_DIABLO_SOURCES
    "${SOURCE_DIR}/my_diablo_game/g_*.c"
    "${SOURCE_DIR}/my_diablo_game/ai_*.c"
)

list(REMOVE_ITEM QAGAME_DIABLO_SOURCES
    "${SOURCE_DIR}/my_diablo_game/g_rankings.c"
)

list(APPEND QAGAME_DIABLO_SOURCES
    ${DIABLO_SHARED_MATH}
    "${SOURCE_DIR}/qcommon/q_shared.c"
    ${DIABLO_BG_SOURCES}
    ${DIABLO_HEIGHTMAP_SOURCES}
    "${SOURCE_DIR}/my_diablo_game/g_syscalls.c"
)

add_library(qagame_diablo SHARED ${QAGAME_DIABLO_SOURCES})
target_compile_definitions(qagame_diablo PRIVATE QAGAME)
target_include_directories(qagame_diablo PRIVATE "${SOURCE_DIR}/my_diablo_heightmap")
target_link_libraries(qagame_diablo PRIVATE ${COMMON_LIBRARIES})
set_target_properties(qagame_diablo PROPERTIES OUTPUT_NAME "qagame")
set_output_dirs(qagame_diablo SUBDIRECTORY "my_diablo_output")

# --- CGAME ---
file(GLOB CGAME_DIABLO_SOURCES
    "${SOURCE_DIR}/my_diablo_cgame/cg_*.c"
)

list(REMOVE_ITEM CGAME_DIABLO_SOURCES
    "${SOURCE_DIR}/my_diablo_cgame/cg_syscalls.c"
    "${SOURCE_DIR}/my_diablo_cgame/cg_newdraw.c"
)

list(APPEND CGAME_DIABLO_SOURCES
    ${DIABLO_SHARED_MATH}
    "${SOURCE_DIR}/qcommon/q_shared.c"
    "${SOURCE_DIR}/cgame/cg_syscalls.c"
    ${DIABLO_BG_SOURCES}
)

add_library(cgame_diablo SHARED ${CGAME_DIABLO_SOURCES})
target_compile_definitions(cgame_diablo PRIVATE CGAME)
target_include_directories(cgame_diablo PRIVATE "${SOURCE_DIR}/my_diablo_game")
target_link_libraries(cgame_diablo PRIVATE ${COMMON_LIBRARIES})
set_target_properties(cgame_diablo PROPERTIES OUTPUT_NAME "cgame")
set_output_dirs(cgame_diablo SUBDIRECTORY "my_diablo_output")

# --- UI ---
file(GLOB UI_DIABLO_SOURCES
    "${SOURCE_DIR}/my_diablo_ui/ui_*.c"
)

list(REMOVE_ITEM UI_DIABLO_SOURCES
    "${SOURCE_DIR}/my_diablo_ui/ui_syscalls.c"
)

list(FILTER UI_DIABLO_SOURCES EXCLUDE REGEX ".*ui_login.*")
list(FILTER UI_DIABLO_SOURCES EXCLUDE REGEX ".*ui_rankings.*")
list(FILTER UI_DIABLO_SOURCES EXCLUDE REGEX ".*ui_rankstatus.*")
list(FILTER UI_DIABLO_SOURCES EXCLUDE REGEX ".*ui_signup.*")
list(FILTER UI_DIABLO_SOURCES EXCLUDE REGEX ".*ui_specifyleague.*")

list(APPEND UI_DIABLO_SOURCES
    ${DIABLO_SHARED_MATH}
    "${SOURCE_DIR}/ui/ui_syscalls.c"
    "${SOURCE_DIR}/my_diablo_game/bg_misc.c"
    "${SOURCE_DIR}/my_diablo_game/bg_lib.c"
    "${SOURCE_DIR}/qcommon/q_shared.c"
)

add_library(ui_diablo SHARED ${UI_DIABLO_SOURCES})
target_compile_definitions(ui_diablo PRIVATE UI)
target_link_libraries(ui_diablo PRIVATE ${COMMON_LIBRARIES})
set_target_properties(ui_diablo PROPERTIES OUTPUT_NAME "ui")
set_output_dirs(ui_diablo SUBDIRECTORY "my_diablo_output")

# QVM sources for my_diablo mod
# QVM targets for my_diablo mod - DISABLED (DLL-only mod)
# The Diablo mod uses custom headers (json.h, stb_image_stub.h) that cannot compile to QVM.
# We only need DLL modules (cgame.dll, qagame.dll, ui.dll).
#
# set(CGAME_DIABLO_QVM_SOURCES "${SOURCE_DIR}/my_diablo_cgame/cg_syscalls.asm")
# set(QAGAME_DIABLO_QVM_SOURCES "${SOURCE_DIR}/my_diablo_game/g_syscalls.asm")
# set(UI_DIABLO_QVM_SOURCES "${SOURCE_DIR}/ui/ui_syscalls.asm")
#
# if(BUILD_GAME_QVMS)
#     add_qvm(cgameqvm_diablo
#         DEFINITIONS CGAME
#         OUTPUT_NAME cgame
#         OUTPUT_DIRECTORY my_diablo_output/vm
#         SOURCES ${CGAME_DIABLO_SOURCES} ${CGAME_DIABLO_QVM_SOURCES})
#     
#     add_qvm(qagameqvm_diablo
#         DEFINITIONS QAGAME
#         OUTPUT_NAME qagame
#         OUTPUT_DIRECTORY my_diablo_output/vm
#         SOURCES ${QAGAME_DIABLO_SOURCES} ${QAGAME_DIABLO_QVM_SOURCES})
#     
#     add_qvm(uiqvm_diablo
#         DEFINITIONS UI
#         OUTPUT_NAME ui
#         OUTPUT_DIRECTORY my_diablo_output/vm
#         SOURCES ${UI_DIABLO_SOURCES} ${UI_DIABLO_QVM_SOURCES})
# endif()
