# Instrukcja Uruchamiania Gry - Diablo Mod

## ⚠️ WAŻNE: Ustawienia VM (Virtual Machine)

### Dlaczego potrzebne są ustawienia +set vm_* 0?

Quake III Arena ma **dwa systemy ładowania modułów**:

1. **QVM (Quake Virtual Machine)** - Domyślny tryb
   - Ładuje pliki `.qvm` z `pak0.pk3`
   - Bezpieczny, ale **NIE ZAWIERA NASZYCH MODYFIKACJI**
   - Numeracja: `vm_* = 2` (domyślna)

2. **Native DLL** - Tryb developerski
   - Ładuje natywne pliki `.dll` z `my_diablo_output/`
   - **Zawiera wszystkie nasze zmiany** (skybox, world definition, terrain)
   - Numeracja: `vm_* = 0` (wymusza DLL)

### Co się dzieje bez tych ustawień:

```
❌ Gra ładuje: vm/cgame.qvm z pak0.pk3 (oryginalny kod)
❌ Gra ładuje: vm/ui.qvm z pak0.pk3 (oryginalny kod)
❌ Gra ładuje: vm/qagame.qvm z pak0.pk3 (oryginalny kod)

REZULTAT: Skybox nie działa, world definition nie działa, wszystkie nasze zmiany są IGNOROWANE!
```

### Co się dzieje Z tymi ustawieniami:

```
✅ Gra ładuje: my_diablo_output/cgame.dll (nasz kod)
✅ Gra ładuje: my_diablo_output/ui.dll (nasz kod)
✅ Gra ładuje: my_diablo_output/qagame.dll (nasz kod z skybox)

REZULTAT: Wszystkie nasze modyfikacje działają poprawnie!
```

## Prawidłowa Komenda Uruchamiania

### Wersja Pełna (Z Wszystkimi Ustawieniami):

```powershell
cd C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output

.\ioquake3_diablo.exe `
  +set fs_basepath "C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug" `
  +set fs_game my_diablo_output `
  +set sv_pure 0 `
  +set vm_cgame 0 `
  +set vm_ui 0 `
  +set vm_game 0 `
  +set developer 1 `
  +set logfile 2 `
  +world my_level
```

### Wersja Skrócona (Jednolinijkowa):

```powershell
cd C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output; .\ioquake3_diablo.exe +set fs_basepath "C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug" +set fs_game my_diablo_output +set sv_pure 0 +set vm_cgame 0 +set vm_ui 0 +set vm_game 0 +set developer 1 +set logfile 2 +world my_level
```

## Wyjaśnienie Każdego Ustawienia

| Ustawienie | Wartość | Opis | **Czy potrzebne?** |
|------------|---------|------|---------------------|
| `fs_basepath` | `build/Debug` | Główny folder z pak0.pk3 | ✅ **TAK** - bez tego nie znajdzie plików bazowych |
| `fs_game` | `my_diablo_output` | Folder z naszym modem | ✅ **TAK** - wskazuje gdzie są nasze DLL |
| `sv_pure 0` | `0` | Wyłącza sprawdzanie czystości plików | ✅ **TAK** - bez tego nie załaduje modyfikowanych DLL |
| `vm_cgame 0` | `0` | Wymusza cgame.dll zamiast cgame.qvm | ✅ **KRYTYCZNE** - inaczej nasze zmiany w grafice nie działają |
| `vm_ui 0` | `0` | Wymusza ui.dll zamiast ui.qvm | ✅ **KRYTYCZNE** - inaczej nasze zmiany w UI nie działają |
| `vm_game 0` | `0` | Wymusza qagame.dll zamiast qagame.qvm | ✅ **KRYTYCZNE** - inaczej skybox i world definition nie działają! |
| `developer 1` | `1` | Włącza komunikaty debug | 🔧 Opcjonalne (ale przydatne do testowania) |
| `logfile 2` | `2` | Zapisuje log do qconsole.log | 🔧 Opcjonalne (ale przydatne do debugowania) |

## Weryfikacja Czy Ładują Się Nasze DLL

Po uruchomieniu gry sprawdź log:

```powershell
Get-Content C:\Users\AI\AppData\Roaming\Quake3\my_diablo_output\qconsole.log | Select-String -Pattern "Loading.*\.dll"
```

### Prawidłowy Output (DOBRY ✅):

```
Loading DLL file: C:\...\my_diablo_output\cgame.dll
Loading DLL file: C:\...\my_diablo_output\ui.dll
Loading DLL file: C:\...\my_diablo_output\qagame.dll
```

### Nieprawidłowy Output (ZŁY ❌):

```
Loading vm file vm/cgame.qvm...
Loading vm file vm/ui.qvm...
Loading vm file vm/qagame.qvm...
```

Jeśli widzisz `.qvm` - **ustawienia vm_* nie działają** i gra używa oryginalnego kodu!

## Skrót: Szybkie Uruchomienie

Stwórz plik `run_diablo.bat`:

```batch
@echo off
cd /d "C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output"
start ioquake3_diablo.exe +set fs_basepath "C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug" +set fs_game my_diablo_output +set sv_pure 0 +set vm_cgame 0 +set vm_ui 0 +set vm_game 0 +set developer 1 +set logfile 2 +world my_level
```

Teraz możesz uruchamiać grę jednym kliknięciem!

## Podsumowanie

| Pytanie | Odpowiedź |
|---------|-----------|
| Czy ustawienia `vm_*` są potrzebne? | **TAK, ABSOLUTNIE!** Bez nich gra ładuje oryginalne QVM z pak0.pk3 |
| Co się ładuje teraz? | **Nasze DLL** - cgame.dll, ui.dll, qagame.dll z my_diablo_output/ |
| Co się ładowałoby bez vm_* 0? | **Oryginalne QVM** - cgame.qvm, ui.qvm, qagame.qvm z pak0.pk3 |
| Czy skybox działa bez vm_game 0? | **NIE!** Kod skybox jest w qagame.dll, bez vm_game 0 używa oryginalnego qagame.qvm |
| Czy można pominąć któreś ustawienie? | **NIE!** Wszystkie trzy (vm_cgame, vm_ui, vm_game) muszą być = 0 |

---

**Dla Ciekawskich:** Sprawdź wartości domyślne:

```
vm_cgame 2   (2 = QVM, 1 = JIT compiled QVM, 0 = native DLL)
vm_ui 2
vm_game 2
```

Wartość `2` to "interpreted QVM" - najbezpieczniejsza, ale **nie zawiera naszych zmian**!
