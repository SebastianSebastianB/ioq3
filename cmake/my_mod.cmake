# cmake/my_mod.cmake

message(STATUS "--- Defining build targets for My Awesome Mod ---")

# --- Logika serwera (qagame) dla Twojego moda ---
file(GLOB QAGAME_MYMOD_SOURCES
    "${SOURCE_DIR}/my_mod/g_*.c"
    "${SOURCE_DIR}/my_mod/ai_*.c"
    "${SOURCE_DIR}/my_mod/bg_*.c"
)
add_library(qagame_mymod SHARED ${QAGAME_MYMOD_SOURCES})
target_link_libraries(qagame_mymod PRIVATE qcommon)

# --- Logika klienta (cgame) dla Twojego moda ---
file(GLOB CGAME_MYMOD_SOURCES
    "${SOURCE_DIR}/my_mod/cg_*.c"
    "${SOURCE_DIR}/my_mod/bg_*.c"
    "${SOURCE_DIR}/my_mod/ui_shared.c" # Niektóre pliki są wspólne
)
add_library(cgame_mymod SHARED ${CGAME_MYMOD_SOURCES})
target_link_libraries(cgame_mymod PRIVATE qcommon)

# --- Logika menu (ui) dla Twojego moda ---
file(GLOB UI_MYMOD_SOURCES
    "${SOURCE_DIR}/my_mod/ui_*.c"
    "${SOURCE_DIR}/my_mod/bg_*.c"
)
add_library(ui_mymod SHARED ${UI_MYMOD_SOURCES})
target_link_libraries(ui_mymod PRIVATE qcommon)