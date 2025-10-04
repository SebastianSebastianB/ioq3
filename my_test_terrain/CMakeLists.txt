cmake_minimum_required(VERSION 3.10)
project(MyTestTerrain)

# Ustaw standard C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Znajdź OpenGL
find_package(OpenGL REQUIRED)

# Znajdź GLFW (najpierw sprawdź czy jest zainstalowany)
# Jeśli nie ma GLFW, trzeba będzie go doinstalować lub użyć wersji ze źródeł
find_package(glfw3 QUIET)

if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found, will use Windows native OpenGL context")
    set(USE_NATIVE_OPENGL ON)
else()
    message(STATUS "GLFW found, using GLFW for window management")
    set(USE_NATIVE_OPENGL OFF)
endif()

# Pliki źródłowe
set(SOURCES
    src/main.cpp
    src/terrain_loader.cpp
    src/renderer.cpp
    src/player.cpp
    src/collision.cpp
)

# Pliki nagłówkowe
set(HEADERS
    src/terrain_loader.h
    src/renderer.h
    src/player.h
    src/collision.h
    src/types.h
)

# Utwórz executable
add_executable(test_terrain ${SOURCES} ${HEADERS})

# Linkowanie bibliotek
target_link_libraries(test_terrain 
    OpenGL::GL
    glu32
    opengl32
)

if(NOT USE_NATIVE_OPENGL)
    target_link_libraries(test_terrain glfw)
endif()

# Windows-specific
if(WIN32)
    target_link_libraries(test_terrain 
        gdi32
        user32
        kernel32
    )
    
    # Ustaw subsystem na Windows (nie Console)
    set_target_properties(test_terrain PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
    
    # Ustaw working directory na katalog z mapą
    set_target_properties(test_terrain PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../build/Debug/my_diablo_output"
    )
endif()

# Dodaj definicje
if(USE_NATIVE_OPENGL)
    target_compile_definitions(test_terrain PRIVATE USE_NATIVE_OPENGL)
endif()

# Output directory
set_target_properties(test_terrain PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
