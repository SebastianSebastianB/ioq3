# Terrain Collision Test - Szybki Przewodnik

## 🎯 Cel Projektu

Niezależny, minimalistyczny silnik 3D do **izolowanego testowania systemu kolizji terenu**. 
Całkowicie oddzielony od Quake III Arena, dzięki czemu możemy:
- Testować algorytmy kolizji w czystej formie
- Szybko iterować (kompilacja ~10 sekund)
- Łatwo debugować (brak skomplikowanej infrastruktury Q3)
- Weryfikować czy algorytm działa, zanim zintegrujemy go z Q3

## 📁 Struktura Plików

```
my_test_terrain/
├── CMakeLists.txt              # Konfiguracja CMake
├── README.md                   # Dokumentacja podstawowa
├── ARCHITECTURE.md             # Szczegóły techniczne
├── TEST_INSTRUCTIONS.md        # Instrukcja testowania
├── rebuild.bat                 # Szybka rekompilacja
├── run_test.bat                # Uruchomienie programu
├── build/                      # Katalog buildowy (generowany)
└── src/
    ├── main.cpp               # Pętla gry + okno Win32 + OpenGL
    ├── types.h                # Vec3, TerrainData, stałe
    ├── terrain_loader.h/cpp   # Ładowanie .world, GetHeightAt, GetNormalAt
    ├── collision.h/cpp        # GroundTrace (główna funkcja kolizji)
    ├── player.h/cpp           # Fizyka gracza, input, update
    └── renderer.h/cpp         # Rendering OpenGL (niebo + teren)
```

## 🚀 Kompilacja i Uruchomienie

### Pierwsza kompilacja:
```bash
cd my_test_terrain
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Debug
```

### Kolejne kompilacje:
```bash
cd my_test_terrain
rebuild.bat
```

### Uruchomienie:

**Metoda 1: Domyślna mapa (my_level3.world)**
```bash
cd my_test_terrain
run_test.bat
```

**Metoda 2: Własna mapa**
```bash
cd my_test_terrain
run_custom.bat C:\sciezka\do\mojej_mapy.world
```

**Metoda 3: Bezpośrednio z linii poleceń**
```bash
cd my_test_terrain
build\bin\Debug\test_terrain.exe "C:\sciezka\do\mapy.world"
```

Jeśli nie podasz parametru, program użyje domyślnej ścieżki: `maps/my_level3.world`

## 🎮 Sterowanie

| Klawisz | Akcja |
|---------|-------|
| W/A/S/D | Ruch (przód/lewo/tył/prawo) |
| SPACJA | Skok |
| LPM (przytrzymaj) | Przejęcie kontroli myszy (rozglądanie) |
| PPM | Zwolnienie myszy |
| ESC | Wyjście |

## ✅ Co Testujemy

1. **Podstawowe chodzenie** - Czy gracz płynnie chodzi po terenie bez "utykania"?
2. **Łagodne zbocza** - Czy można wchodzić na wzgórza < 45°?
3. **Strome zbocza** - Czy gracz zsuwa się ze stromych zboczy > 45°?
4. **Skok** - Czy skok działa poprawnie?
5. **Spadanie** - Czy grawitacja działa, czy gracz ląduje na terenie?

## 🔍 Kluczowe Funkcje

### `Collision::GroundTrace()`
Główna funkcja kolizji - odpowiednik `PM_GroundTrace` z Q3.
```cpp
GroundTraceResult GroundTrace(position, minZ, velocity) {
    terrainHeight = GetHeightAt(position);
    playerBottom = position.z + minZ;
    
    if (playerBottom < terrainHeight + TOLERANCE && velocity.z <= 0) {
        normal = GetNormalAt(position);
        
        if (normal.z < MIN_WALK_NORMAL) {
            return {onGround: true, canWalk: false, tooSteep: true};
        } else {
            return {onGround: true, canWalk: true};
        }
    }
    return {onGround: false};
}
```

### `TerrainLoader::GetHeightAt()`
Interpolacja biliniowa wysokości terenu.

### `TerrainLoader::GetNormalAt()`
Oblicza normalną powierzchni (4-punktowe próbkowanie).

## 📊 Diagnostyka

Program wypisuje na konsoli:
```
=== Terrain Collision Test ===
Ładowanie terenu z: maps/my_level3.world
Parametry terenu:
  width: 257
  height: 257
  cellSize: 32.0
  verticalScale: 1.0

Pozycja startowa gracza: (4112.0, 4112.0, 200.0)

=== STEROWANIE ===
[...]

NA ZIEMI - pos.z: 42.5, terrain: 18.5, normal.z: 0.99
ZBOCZE ZBYT STROME! normal.z = 0.65 (min: 0.70)
ŚLIZGANIE PO STROMYM ZBOCZU!
```

## 🔧 Kluczowe Parametry

| Parametr | Wartość | Opis |
|----------|---------|------|
| `MIN_WALK_NORMAL` | 0.7 | Minimalna normalna Z (cos 45° ≈ 0.707) |
| `GROUND_STICK_TOLERANCE` | 4.0 | Strefa przyciągania do podłoża |
| `GRAVITY` | 800.0 | Przyspieszenie grawitacyjne |
| `MOVE_SPEED` | 320.0 | Prędkość chodzenia |
| `JUMP_VELOCITY` | 270.0 | Prędkość skoku |

## 🎯 Interpretacja Wyników

### ✅ Jeśli wszystko działa poprawnie:
**Algorytm kolizji jest OK!** Problem w Quake III leży w:
- Integracji z systemem BSP
- Kolejności wywołań funkcji
- Innych systemach Q3 (PM_AirMove, PM_WalkMove, itp.)

### ❌ Jeśli są problemy:
**Algorytm wymaga poprawek.** Możemy:
- Łatwo debugować w izolacji
- Testować różne warianty
- Wizualizować normalne, pozycje, prędkości
- Eksperymentować z parametrami

## 📚 Dokumentacja

- `README.md` - Podstawowe informacje
- `ARCHITECTURE.md` - Szczegóły techniczne, algorytmy, metryki
- `TEST_INSTRUCTIONS.md` - Dokładna instrukcja testowania
- Ten plik - Szybki przegląd

## 🔄 Workflow

1. **Testuj** w tym silniku
2. **Zweryfikuj** że algorytm działa
3. **Skopiuj** dokładny kod do Quake III
4. **Porównaj** zachowanie (dodaj identyczne logi)
5. **Znajdź** różnice

## 💡 Przewagi tego podejścia

- ✅ Szybka kompilacja (~10s vs ~2min Q3)
- ✅ Prosty debugging (brak skomplikowanej infrastruktury)
- ✅ Pełna kontrola nad kodem
- ✅ Łatwe eksperymenty
- ✅ Izolacja problemu
- ✅ Czyste testy algorytmu

## 🛠️ Znane Ograniczenia

- ❌ Tylko heightfield (brak BSP)
- ❌ Brak kolizji ze ścianami/obiektami
- ❌ Brak tekstur (jednolite kolory)
- ❌ Uproszczona fizyka

**To jest celowe!** Chcemy testować TYLKO kolizję z terenem.

---

**Sukces projektu = Odpowiedź na pytanie:**  
*"Czy sam algorytm kolizji terenu działa poprawnie?"*

Jeśli TAK → Problem jest w integracji z Q3  
Jeśli NIE → Możemy łatwo poprawić algorytm tutaj
