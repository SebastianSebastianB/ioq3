# ✅ GOTOWE - System Uruchamiania Terrain Test Engine

## 🎯 Zmiany

Program teraz **akceptuje ścieżkę do mapy jako parametr wiersza poleceń**!

## 🚀 Jak Uruchomić

### Metoda 1: PowerShell (ZALECANA)
```powershell
cd my_test_terrain
powershell -ExecutionPolicy Bypass -File .\run_test.ps1
```

### Metoda 2: CMD (Batch)
```cmd
cd my_test_terrain
run_test.bat
```

### Metoda 3: Bezpośrednio z parametrem
```powershell
.\build\bin\Debug\test_terrain.exe "C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output\maps\my_level3.world"
```

### Metoda 4: Własna mapa
```cmd
run_custom.bat "C:\sciezka\do\twojej_mapy.world"
```

## 📁 Dostępne Skrypty

| Skrypt | Opis |
|--------|------|
| `run_test.ps1` | PowerShell launcher (domyślna mapa) |
| `run_test.bat` | CMD launcher (domyślna mapa) |
| `run_custom.bat` | CMD launcher (własna mapa) |
| `rebuild.bat` | Szybka rekompilacja |

## ⚙️ Jak Działa

1. Program sprawdza `argc` (liczbę argumentów)
2. Jeśli `argc >= 2` → używa `argv[1]` jako ścieżki do mapy
3. Jeśli brak argumentu → używa domyślnej ścieżki: `maps/my_level3.world`

### Output Diagnostyczny

Program wypisuje na konsoli:
```
=== Terrain Collision Test ===
Argumenty: 2
  argv[0]: C:\...\test_terrain.exe
  argv[1]: C:\...\my_level3.world

Używam mapy z argumentu: C:\...\my_level3.world

Ładowanie terenu z: C:\...\my_level3.world
Parametry terenu:
  width: 257
  height: 257
  cellSize: 32.0
  verticalScale: 1.0
```

## 🔧 Konfiguracja Ścieżek

### Domyślna ścieżka w `run_test.ps1` i `run_test.bat`:
```
C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output\maps\my_level3.world
```

### Aby zmienić:
Edytuj linię w `run_test.ps1`:
```powershell
$MapPath = "TWOJA_SCIEZKA_TUTAJ"
```

Lub w `run_test.bat`:
```batch
set MAP_PATH=TWOJA_SCIEZKA_TUTAJ
```

## ✅ Weryfikacja

Program został pomyślnie:
- ✅ Skompilowany z obsługą parametrów
- ✅ Zintegrowany ze skryptami uruchomieniowymi
- ✅ Przetestowany (uruchamia się poprawnie)

## 🎮 Następne Kroki

Teraz możesz:
1. **Uruchomić program** i przetestować system kolizji
2. **Sprawdzić czy "utykanie" występuje** w izolowanym silniku
3. **Testować różne mapy** przekazując różne pliki `.world`
4. **Porównać zachowanie** z Quake III Arena

---

**Program jest gotowy do testowania!** 🚀
