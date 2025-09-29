Jasne, oto szczegółowy plan działania w formacie Markdown. Uwzględnia on Twoją architekturę z osobnymi folderami na modyfikacje i rozbija cały proces na logiczne, zarządzalne fazy.

---

# Mapa Drogowa: Implementacja Silnika Terenu w ioquake3

## Cel Projektu

Stworzenie nowego, równoległego systemu renderowania i fizyki w silniku ioquake3, zdolnego do obsługi map generowanych z heightmap. Celem końcowym jest stworzenie gry FPP w stylu Diablo 2, a pierwszym krokiem jest działający prototyp technologiczny.

## Architektura Projektu

- **Silnik Renderujący (`renderer_terrain.dll`):** Całkowicie nowa, osobna biblioteka DLL odpowiedzialna za rysowanie terenu i skyboxa.
- **Silnik Fizyki (`cm_terrain.c`):** Nowy zestaw funkcji w `qcommon` do obsługi kolizji ze światem opartym na heightmapie.
- **Modyfikacja Gry (`my_diablo_mod`):** Logika gry, która wykorzystuje nowy silnik. Użyjemy Twojej struktury:
  - `code/my_diablo_cgame/`
  - `code/my_diablo_qagame/`
  - `code/my_diablo_ui/`

---

## Faza 0: Przygotowanie Fundamentów

**Cel:** Przygotować system budowania i strukturę projektu do pracy nad nowymi komponentami silnika.

-   [ ] **Utworzenie Celu Kompilacji dla Renderera Terenu:**
    -   [ ] W folderze `code/` stwórz nowy podfolder `renderer_terrain/`.
    -   [ ] W folderze `cmake/` stwórz nowy plik `renderer_terrain.cmake`.
    -   [ ] W głównym `CMakeLists.txt` dodaj opcję `option(BUILD_RENDERER_TERRAIN "Build the Terrain Renderer" ON)`.
    -   [ ] W głównym `CMakeLists.txt` dodaj warunkowe dołączenie: `if(BUILD_RENDERER_TERRAIN) include(renderer_terrain) endif()`.
    -   [ ] Wypełnij `renderer_terrain.cmake` podstawową definicją `add_library(renderer_terrain SHARED ...)` z jednym pustym plikiem `.c`, aby upewnić się, że kompiluje się pusta biblioteka `.dll`.

-   [ ] **Utworzenie Plików dla Nowej Fizyki:**
    -   [ ] W `code/qcommon/` stwórz dwa nowe pliki: `cm_terrain.c` (pusty) i `cm_terrain.h` (zabezpieczony include guardem).
    -   [ ] Dodaj `cm_terrain.c` do listy kompilacji celu `qcommon` w pliku `cmake/server.cmake`.

-   [ ] **Przygotowanie Modyfikacji Gry:**
    -   [ ] Skopiuj istniejący, działający mod (`my_mod`) do nowych folderów: `my_diablo_cgame`, `my_diablo_qagame`, `my_diablo_ui`.
    -   [ ] Stwórz nowy plik `cmake/my_diablo_mod.cmake` i skonfiguruj go tak, aby kompilował trzy nowe biblioteki DLL (`qagame_diablo.dll` itd.) z nowych folderów.
    -   [ ] Dodaj opcję `BUILD_DIABLO_MOD` do głównego `CMakeLists.txt`.

---

## Faza 1: Pierwszy Obraz na Ekranie (Bez Fizyki)

**Cel:** Uruchomić grę z niestandardową "mapą" i zobaczyć wyrenderowany, oświetlony teren oraz skybox. Gracz będzie mógł latać przez scenę w trybie `noclip`.

-   [ ] **Implementacja Renderera Terenu (`renderer_terrain.dll`):**
    -   [ ] **Wczytywanie Heightmapy:**
        -   [ ] Zintegruj bibliotekę `stb_image.h` ze swoim projektem (po prostu dodaj plik do źródeł renderera).
        -   [ ] Napisz funkcję `Terrain_LoadHeightmap(const char *filename)`, która wczyta plik `.jpg` i stworzy w pamięci tablicę `float height[width][height]`.
    -   [ ] **Generowanie Siatki (Mesh):**
        -   [ ] Napisz funkcję `Terrain_GenerateMesh()`, która na podstawie tablicy wysokości stworzy tablice wierzchołków (`vertices`), normalnych (`normals`) i indeksów (`indices`).
        -   [ ] Napisz funkcję `Terrain_UploadToGPU()`, która załaduje te dane do obiektów VBO (Vertex Buffer Object) i VAO (Vertex Array Object) w OpenGL.
    -   [ ] **Renderowanie Skyboxa:**
        -   [ ] Napisz prosty kod renderujący sześcian z 6 teksturami, który jest zawsze wyśrodkowany na pozycji kamery.
    -   [ ] **Główna Pętla Renderowania:**
        -   [ ] Stwórz główną funkcję `RE_RenderScene()`, która w każdej klatce będzie czyścić bufor, rysować skybox, a następnie rysować siatkę terenu za pomocą `glDrawElements`.
        -   [ ] Zaimplementuj proste oświetlenie kierunkowe (symulujące słońce) za pomocą prostego shadera GLSL.

-   [ ] **Integracja z Klientem Gry (`my_diablo_cgame`):**
    -   [ ] **Mechanizm Ładowania Mapy:**
        -   [ ] W `code/client/cl_main.c` zmodyfikuj logikę ładowania mapy. Dodaj warunek, który sprawdzi, czy nazwa mapy kończy się na specjalne rozszerzenie (np. `.hmap`).
        -   [ ] Jeśli tak, klient ma dynamicznie załadować `renderer_terrain.dll` zamiast `renderer_gl2.dll`.
    -   [ ] **Przekazywanie Danych:**
        -   [ ] Zmodyfikuj kod klienta tak, aby przekazywał pozycję kamery i wektory widoku do Twojego nowego renderera w każdej klatce.

---

## Faza 2: Ożywienie Świata (Fizyka i Kolizje)

**Cel:** Sprawić, by gracz i pociski kolidowały z terenem.

-   [ ] **Implementacja Fizyki Terenu (`cm_terrain.c`):**
    -   [ ] **Przekazanie Danych:** Renderer musi udostępnić tablicę wysokości (`float height[][]`) dla systemu fizyki.
    -   [ ] **Funkcja Wysokości:** Zaimplementuj `float CM_Terrain_GetHeight(vec3_t point)`, która zwraca interpolowaną wysokość terenu w danym punkcie.
    -   [ ] **Funkcja Śledzenia Promienia (Raycasting):** Zaimplementuj `void CM_Terrain_Trace(trace_t *results, const vec3_t start, const vec3_t end, ...)` - to najtrudniejsza część. Musi ona sprawdzać przecięcie wektora z siatką trójkątów terenu. Można zacząć od uproszczonej wersji, która sprawdza tylko wysokość.

-   [ ] **Integracja z Głównym Systemem Kolizji (`qcommon`):**
    -   [ ] W `code/qcommon/cm_trace.c` stwórz wskaźniki funkcyjne, np. `cm_trace_func`.
    -   [ ] Domyślnie wskaźnik pokazuje na oryginalną funkcję `CM_Trace`.
    -   [ ] Gdy Twoja mapa `.hmap` jest ładowana, przestaw ten wskaźnik, aby pokazywał na Twoją nową funkcję `CM_Terrain_Trace`.

-   [ ] **Fizyka Gracza (`my_diablo_qagame`):**
    -   [ ] W pliku `g_phys.c` (w folderze `my_diablo_qagame`) zmodyfikuj funkcję `G_RunPlayerMove`.
    -   [ ] Na końcu funkcji, po obliczeniu nowej pozycji gracza, wywołaj `CM_Terrain_GetHeight` i jeśli gracz jest pod ziemią, "przyklej" go do powierzchni.

---

## Faza 3: Pierwsza Grywalna Pętla

**Cel:** Połączyć wszystkie elementy w działający prototyp, w którym można biegać i strzelać.

-   [ ] **Spawnowanie Gracza (`my_diablo_qagame`):**
    -   [ ] W `g_client.c` zmodyfikuj funkcję `ClientBegin`.
    -   [ ] Ustaw pozycję startową gracza na środku mapy (`map_width / 2`, `map_height / 2`).
    -   [ ] Użyj `CM_Terrain_GetHeight`, aby znaleźć prawidłową wysokość startową `Y` i uniknąć wpadnięcia pod mapę.

-   [ ] **Testowanie Broni:**
    -   [ ] **Broń typu "hitscan" (strzelba, karabin maszynowy):** Powinny działać automatycznie, ponieważ używają one teraz Twojej nowej funkcji `CM_Terrain_Trace`. Sprawdź, czy ślady po kulach pojawiają się na terenie.
    -   [ ] **Broń z pociskami (rakietnica):**
        -   [ ] W `g_missile.c` (w `my_diablo_qagame`) znajdź logikę poruszania pociskami.
        -   [ ] Upewnij się, że ona również używa `CM_Terrain_Trace` do sprawdzania kolizji, a nie starego systemu.

-   [ ] **Refaktoryzacja i Czyszczenie:**
    -   [ ] Przejrzyj kod i upewnij się, że przełączanie między starym a nowym silnikiem jest czyste i nie powoduje problemów.
    -   [ ] Dodaj podstawowe komunikaty o błędach (np. gdy nie uda się wczytać heightmapy).

---