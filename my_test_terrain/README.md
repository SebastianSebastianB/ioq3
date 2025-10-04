# Terrain Collision Test - Niezależny Silnik Testowy

## Opis

Minimalistyczny, niezależny silnik 3D do testowania systemu kolizji z terenem heightfield.
Całkowicie oddzielony od Quake III Arena - pozwala na izolowane testowanie algorytmów kolizji.

## Funkcje

- ✅ Ładowanie mapy terenu z pliku `.world`
- ✅ Renderowanie terenu OpenGL z prostym oświetleniem
- ✅ System kolizji gracza z terenem (identyczny jak w Q3)
- ✅ Detekcja stromych zboczy (MIN_WALK_NORMAL)
- ✅ Grawitacja i fizyka ruchu
- ✅ Pełna kontrola kamerą (FPS-style)
- ✅ Niebo jako jednolity kolor niebieski

## Kompilacja

### Windows (Visual Studio)

```bash
cd my_test_terrain
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Debug
```

Lub otwórz wygenerowany plik `.sln` w Visual Studio.

### Uruchomienie

**Metoda 1: Używając domyślnej mapy (my_level3.world)**
```bash
cd my_test_terrain
run_test.bat
```

**Metoda 2: Używając własnej mapy**
```bash
cd my_test_terrain
run_custom.bat "C:\path\to\your_map.world"
```

**Metoda 3: Bezpośrednio z parametrem**
```bash
test_terrain.exe "maps/my_level3.world"
```

Program automatycznie wykryje, czy podano ścieżkę do mapy jako argument.
Jeśli nie, użyje domyślnej ścieżki: `maps/my_level3.world`

**WAŻNE:** Ścieżka może być:
- Bezwzględna: `C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output\maps\my_level3.world`
- Względna: `maps/my_level3.world` (względem working directory)

## Sterowanie

- **W/A/S/D** - Ruch (przód/lewo/tył/prawo)
- **SPACJA** - Skok
- **LPM (przytrzymaj)** - Aktywuj kontrolę myszy (rozglądanie się)
- **PPM** - Zwolnij kontrolę myszy
- **ESC** - Wyjście z programu

## Struktura Projektu

```
my_test_terrain/
├── CMakeLists.txt          # Konfiguracja CMake
├── README.md               # Ten plik
└── src/
    ├── main.cpp            # Główna pętla gry + okno Win32
    ├── types.h             # Podstawowe typy (Vec3, TerrainData)
    ├── terrain_loader.h/cpp    # Ładowanie i zapytania heightmap
    ├── collision.h/cpp         # System kolizji (GroundTrace)
    ├── player.h/cpp            # Gracz (fizyka, input, update)
    └── renderer.h/cpp          # Rendering OpenGL
```

## Architektura

### Ładowanie Terenu
`TerrainLoader` czyta plik `.world` i wypełnia strukturę `TerrainData`:
- Szerokość i wysokość siatki
- Rozmiar komórki (`cellSize`)
- Skala pionowa (`verticalScale`)
- Tablica wysokości (`heightMap`)

### Kolizja
`Collision::GroundTrace()` - główna funkcja kolizji:
1. Pobiera wysokość terenu pod graczem
2. Oblicza normalną powierzchni (4-punktowe próbkowanie)
3. Sprawdza czy zbocze jest zbyt strome (`normal.z < MIN_WALK_NORMAL`)
4. Zwraca informacje o kontakcie z podłożem

### Fizyka Gracza
`Player::Update()`:
1. Aplikuje grawitację
2. Aktualizuje pozycję na podstawie prędkości
3. Wywołuje `GroundTrace` do detekcji kolizji
4. Przykleja gracza do terenu lub pozwala mu spadać
5. Obsługuje zsuwanie po stromych zboczach

### Rendering
`Renderer::RenderScene()`:
1. Renderuje niebo (pełnoekranowy quad, kolor RGB: 0.5, 0.7, 1.0)
2. Ustawia kamerę na podstawie pozycji i kątów gracza
3. Renderuje teren jako siatkę trójkątów (`GL_TRIANGLE_STRIP`)
4. Proste oświetlenie kierunkowe

## Debugowanie

Program wypisuje szczegółowe informacje diagnostyczne na konsolę:
- Parametry załadowanego terenu
- Informacje o OpenGL
- Status gracza (pozycja, wysokość terenu, normalna)
- Ostrzeżenia o zbyt stromych zboczach

## Różnice względem Quake III

- ❌ Brak systemu BSP (tylko heightfield)
- ❌ Brak ścian, kolizji z obiektami
- ❌ Brak tekstur (jednolite kolory)
- ❌ Brak zaawansowanego oświetlenia/cieni
- ✅ Identyczny algorytm kolizji terenu
- ✅ Identyczna fizyka gracza

## Cel

Celem tego silnika jest **izolowane testowanie systemu kolizji terenu** bez 
komplikacji związanych z pełnym silnikiem Quake III. Jeśli kolizje działają 
tutaj poprawnie, możemy być pewni, że problem w Q3 leży gdzie indziej.

## Licencja

Ten kod jest testowy i nie zawiera żadnego kodu z Quake III Arena.
