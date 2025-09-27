# cmake/my_mod.cmake
message(STATUS "--- Defining build targets for My Awesome Mod ---")

# --- Wspólne pliki źródłowe (tylko matematyka) ---
set(MYMOD_SHARED_MATH
    "${SOURCE_DIR}/qcommon/q_math.c"
)

# --- Pliki background z my_mod_game (shared files) ---
set(GAME_BG_SOURCES
    "${SOURCE_DIR}/my_mod_game/bg_lib.c"
    "${SOURCE_DIR}/my_mod_game/bg_misc.c" 
    "${SOURCE_DIR}/my_mod_game/bg_pmove.c"
    "${SOURCE_DIR}/my_mod_game/bg_slidemove.c"
)

# --- NOWA STRUKTURA: my_mod_game folder ---  
file(GLOB QAGAME_MYMOD_SOURCES
    "${SOURCE_DIR}/my_mod_game/g_*.c"
    "${SOURCE_DIR}/my_mod_game/ai_*.c"
)

# Debug - pokaż znalezione pliki
message(STATUS "QAGAME GLOB znalazł: ${QAGAME_MYMOD_SOURCES}")

# Usuń problematyczne pliki (NOWE ŚCIEŻKI)
list(REMOVE_ITEM QAGAME_MYMOD_SOURCES 
    "${SOURCE_DIR}/my_mod_game/g_rankings.c"
    "${SOURCE_DIR}/my_mod_game/g_syscalls.c"  # Usuń g_syscalls.c z my_mod_game jeśli istnieje
)

message(STATUS "QAGAME po usunięciu: ${QAGAME_MYMOD_SOURCES}")

# Dodaj potrzebne pliki dla qagame (bez duplikatów)
list(APPEND QAGAME_MYMOD_SOURCES 
    ${MYMOD_SHARED_MATH}
    "${SOURCE_DIR}/qcommon/q_shared.c"
    ${GAME_BG_SOURCES}
    "${SOURCE_DIR}/game/g_syscalls.c"  # Tylko oryginał z game/
)

add_library(qagame_mymod SHARED ${QAGAME_MYMOD_SOURCES})
target_compile_definitions(qagame_mymod PRIVATE QAGAME)
target_link_libraries(qagame_mymod PRIVATE ${COMMON_LIBRARIES})
set_target_properties(qagame_mymod PROPERTIES OUTPUT_NAME "qagame")
set_output_dirs(qagame_mymod SUBDIRECTORY "my_mod_output")

# --- NOWA STRUKTURA: my_mod_cgame folder ---
file(GLOB CGAME_MYMOD_SOURCES
    "${SOURCE_DIR}/my_mod_cgame/cg_*.c"
)

message(STATUS "CGAME GLOB znalazł: ${CGAME_MYMOD_SOURCES}")

# Usuń problematyczne pliki z cgame (NOWE ŚCIEŻKI)
list(REMOVE_ITEM CGAME_MYMOD_SOURCES 
    "${SOURCE_DIR}/my_mod_cgame/cg_syscalls.c"  # Usuń jeśli istnieje w my_mod_cgame
    "${SOURCE_DIR}/my_mod_cgame/cg_newdraw.c"   # Usuń cg_newdraw.c - powoduje duplikaty symboli
)

# Dodaj pliki specyficzne dla cgame
list(APPEND CGAME_MYMOD_SOURCES 
    ${MYMOD_SHARED_MATH}
    "${SOURCE_DIR}/qcommon/q_shared.c"  # Q_* functions
    "${SOURCE_DIR}/cgame/cg_syscalls.c"  # Tylko oryginał z cgame/
    ${GAME_BG_SOURCES}  # bg_* files z my_mod_game
)

add_library(cgame_mymod SHARED ${CGAME_MYMOD_SOURCES})
target_compile_definitions(cgame_mymod PRIVATE CGAME)
# Dodaj include directory dla my_mod_game (shared headers)
target_include_directories(cgame_mymod PRIVATE "${SOURCE_DIR}/my_mod_game")
target_link_libraries(cgame_mymod PRIVATE ${COMMON_LIBRARIES})
set_target_properties(cgame_mymod PROPERTIES OUTPUT_NAME "cgame")
set_output_dirs(cgame_mymod SUBDIRECTORY "my_mod_output")

# --- NOWA STRUKTURA: my_mod_ui folder ---
file(GLOB UI_MYMOD_SOURCES
    "${SOURCE_DIR}/my_mod_ui/ui_*.c"
)

message(STATUS "UI GLOB znalazł: ${UI_MYMOD_SOURCES}")

# Usuń problematyczne pliki z ui (NOWE ŚCIEŻKI)
list(REMOVE_ITEM UI_MYMOD_SOURCES 
    "${SOURCE_DIR}/my_mod_ui/ui_syscalls.c"
)

# Usuń pliki związane z rankingami (powodują błędy kompilacji)
list(FILTER UI_MYMOD_SOURCES EXCLUDE REGEX ".*ui_login.*")
list(FILTER UI_MYMOD_SOURCES EXCLUDE REGEX ".*ui_rankings.*") 
list(FILTER UI_MYMOD_SOURCES EXCLUDE REGEX ".*ui_rankstatus.*")
list(FILTER UI_MYMOD_SOURCES EXCLUDE REGEX ".*ui_signup.*")
list(FILTER UI_MYMOD_SOURCES EXCLUDE REGEX ".*ui_specifyleague.*")

# Dodaj pliki specyficzne dla ui
list(APPEND UI_MYMOD_SOURCES 
    ${MYMOD_SHARED_MATH}
    "${SOURCE_DIR}/ui/ui_syscalls.c"  # Tylko oryginał z ui/
    "${SOURCE_DIR}/my_mod_game/bg_misc.c"    # Dodatkowe funkcje bg_ z my_mod_game
    "${SOURCE_DIR}/my_mod_game/bg_lib.c"     # Biblioteka bg_ z my_mod_game
    "${SOURCE_DIR}/qcommon/q_shared.c" # Wspólne funkcje Q_
)

add_library(ui_mymod SHARED ${UI_MYMOD_SOURCES})
target_compile_definitions(ui_mymod PRIVATE UI)
target_link_libraries(ui_mymod PRIVATE ${COMMON_LIBRARIES})
set_target_properties(ui_mymod PROPERTIES OUTPUT_NAME "ui")
set_output_dirs(ui_mymod SUBDIRECTORY "my_mod_output")