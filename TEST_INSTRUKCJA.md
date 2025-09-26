# Instrukcja testowania moda My Awesome Mod

## Kompilacja zakończona pomyślnie! 

Wszystkie trzy moduły zostały skompilowane:
- ✅ `qagame.dll` - logika gry z cluster rakietami
- ✅ `cgame.dll` - grafika gry z efektami plasma
- ✅ `ui.dll` - interfejs z nowymi rozdzielczościami

## Dodane funkcjonalności:

### 1. Rakiety cluster 
- **Zmiana**: Eksplozja na uderzeniu zamiast czasomierza
- **Bomblets**: Używają grafiki plasma ball zamiast rakiet
- **Zasięg**: Mniejszy obszar rażenia dla lepszej rozgrywki

### 2. Rozdzielczości panoramiczne w UI
Dodane rozdzielczości do Settings → Display:

**16:9 (Widescreen):**
- 1280x720 (HD)
- 1366x768
- 1920x1080 (Full HD)
- 2560x1440 (QHD)

**16:10 (Widescreen):**
- 1280x800
- 1920x1200

**21:9 (Ultrawide):**
- 2560x1080 (Ultrawide Full HD)
- 3440x1440 (Ultrawide QHD)

**32:9 (Super Ultrawide):**
- 3840x1080
- 5120x1440

## Jak przetestować:

### Test 1: Cluster rakiety
1. Uruchom grę w trybie singleplayer
2. Rozpocznij mapę z Rocket Launcher
3. Wystrzel rakietę - powinna eksplodować na uderzeniu
4. Sprawdź, czy bomblets używają efektów plasma (niebieski/fioletowy)

### Test 2: Rozdzielczości panoramiczne  
1. Wejdź do Settings → Display
2. Sprawdź listę rozdzielczości
3. Wybierz jedną z nowych opcji (np. 2560x1080)
4. Potwierdź zmianę - gra powinna przełączyć się na wybraną rozdzielczość

## Pliki DLL:
- Lokalizacja: `build/Debug/my_mod_output/`
- Skopiowane do: `build/Debug/`

## Historia zmian:
1. **CMake**: Naprawiono duplikaty symboli i problemy z kompilacją
2. **g_missile.c**: Zmieniono zachowanie rakiet cluster
3. **ui_video.c**: Dodano obsługę rozdzielczości panoramicznych
4. **Usunięto**: Pliki związane z rankingami (problemy kompilacji)

Powodzenia z testowaniem!