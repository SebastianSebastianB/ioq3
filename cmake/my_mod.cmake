# cmake/my_mod.cmake
message(STATUS "--- Defining build targets for My Awesome Mod ---")

# --- Wspólne pliki źródłowe (tylko matematyka) ---
set(MYMOD_SHARED_MATH
    "${SOURCE_DIR}/qcommon/q_math.c"
)

# --- Pliki background tylko dla qagame (serwer) ---
set(GAME_BG_SOURCES
    "${SOURCE_DIR}/game/bg_lib.c"
    "${SOURCE_DIR}/game/bg_misc.c" 
    "${SOURCE_DIR}/game/bg_pmove.c"
    "${SOURCE_DIR}/game/bg_slidemove.c"
)

# --- Sprawdź co faktycznie znajdzie GLOB dla qagame ---
file(GLOB QAGAME_MYMOD_SOURCES
    "${SOURCE_DIR}/my_mod/g_*.c"
    "${SOURCE_DIR}/my_mod/ai_*.c"
)

# Debug - pokaż znalezione pliki
message(STATUS "QAGAME GLOB znalazł: ${QAGAME_MYMOD_SOURCES}")

# Usuń problematyczne pliki
list(REMOVE_ITEM QAGAME_MYMOD_SOURCES 
    "${SOURCE_DIR}/my_mod/g_rankings.c"
    "${SOURCE_DIR}/my_mod/g_syscalls.c"  # Usuń g_syscalls.c z my_mod jeśli istnieje
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

# --- Sprawdź co faktycznie znajdzie GLOB dla cgame ---
file(GLOB CGAME_MYMOD_SOURCES
    "${SOURCE_DIR}/my_mod/cg_*.c"
)

message(STATUS "CGAME GLOB znalazł: ${CGAME_MYMOD_SOURCES}")

# Usuń problematyczne pliki z cgame
list(REMOVE_ITEM CGAME_MYMOD_SOURCES 
    "${SOURCE_DIR}/my_mod/cg_syscalls.c"  # Usuń jeśli istnieje w my_mod
)

# Dodaj pliki specyficzne dla cgame
list(APPEND CGAME_MYMOD_SOURCES 
    ${MYMOD_SHARED_MATH}
    "${SOURCE_DIR}/cgame/cg_syscalls.c"  # Tylko oryginał z cgame/
)

add_library(cgame_mymod SHARED ${CGAME_MYMOD_SOURCES})
target_compile_definitions(cgame_mymod PRIVATE CGAME)
target_link_libraries(cgame_mymod PRIVATE ${COMMON_LIBRARIES})
set_target_properties(cgame_mymod PROPERTIES OUTPUT_NAME "cgame")
set_output_dirs(cgame_mymod SUBDIRECTORY "my_mod_output")

# --- Sprawdź co faktycznie znajdzie GLOB dla ui ---
file(GLOB UI_MYMOD_SOURCES
    "${SOURCE_DIR}/my_mod/ui_*.c"
)

message(STATUS "UI GLOB znalazł: ${UI_MYMOD_SOURCES}")

# Usuń problematyczne pliki z ui (ui_syscalls.c używamy oryginalny)
list(REMOVE_ITEM UI_MYMOD_SOURCES 
    "${SOURCE_DIR}/my_mod/ui_syscalls.c"
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
    "${SOURCE_DIR}/game/bg_misc.c"    # Dodatkowe funkcje bg_
    "${SOURCE_DIR}/game/bg_lib.c"     # Biblioteka bg_
    "${SOURCE_DIR}/qcommon/q_shared.c" # Wspólne funkcje Q_
)

add_library(ui_mymod SHARED ${UI_MYMOD_SOURCES})
target_compile_definitions(ui_mymod PRIVATE UI)
target_link_libraries(ui_mymod PRIVATE ${COMMON_LIBRARIES})
set_target_properties(ui_mymod PROPERTIES OUTPUT_NAME "ui")
set_output_dirs(ui_mymod SUBDIRECTORY "my_mod_output")