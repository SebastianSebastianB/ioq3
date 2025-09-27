# Test rozdzielczości panoramicznych - instrukcja

## Krok 1: Sprawdź czy opcje są widoczne
1. Uruchom grę z nowym ui.dll
2. Wejdź do Settings → Display  
3. Sprawdź czy w menu "Aspect Ratio" są nowe opcje:
   - 16:9
   - 16:10  
   - 21:9
   - 32:9

## Krok 2: Test zmiany rozdzielczości
1. Wybierz "16:9" w Aspect Ratio
2. Sprawdź czy w liście "Video Mode" pojawiają się opcje 16:9:
   - 1280x720
   - 1366x768
   - 1920x1080
   - 2560x1440

## Krok 3: Zmień rozdzielczość
1. Wybierz np. 1920x1080
2. Kliknij Apply/OK
3. Sprawdź czy gra rzeczywiście przełączyła się na 16:9

## Debug - jeśli nadal nie działa:

### Sprawdź cvars w konsoli:
- `r_mode` - powinno pokazać numer trybu
- `r_customwidth` - jeśli r_mode = -1, to powinna być szerokość
- `r_customheight` - jeśli r_mode = -1, to powinna być wysokość  
- `r_fullscreen` - czy pełny ekran

### Jeśli r_mode pozostaje na starym trybie:
Problem może być w engine - niektóre rozdzielczości mogą wymagać r_mode = -1 i custom width/height.

### Jeśli ratio się nie zmienia:
- Sprawdź czy `vid_restart` pomaga
- Możliwe, że engine ignoruje nowe rozdzielczości

## Poprawka v2 (jeśli potrzebna):
Jeśli rozdzielczości nadal nie działają, będziemy musieli zmusić wszystkie nowe rozdzielczości do używania r_mode = -1 i custom width/height zamiast built-in mode.

## Logi kompilacji:
- **v3**: ui.dll skompilowane (z custom mode logic)
- ✅ Problem z ratio mapping naprawiony (ratios[r] = ratioBuf[i])
- ✅ Dodane proporcje: 21:9 (2.39:1) i 32:9 (3.56:1)  
- ✅ Dodana logika wymuszonego custom mode dla widescreen
- ✅ Fixed resolutions fallback to builtin when not detected

## Zmiany v6:
1. **Custom Mode Logic**: Panoramiczne rozdzielczości zawsze używają r_mode = -1
2. **String Detection**: Sprawdzamy nazwę rozdzielczości zamiast indeksu
3. **Fallback Fix**: Jeśli engine nie wykryje rozdzielczości, używamy builtin list
4. **Static Ratios**: HARDCODED lista aspect ratios - nie zależy od wykrywania
5. **Fixed Mapping**: Poprawione mapowanie static ratio → resolution

## Test v4 - Aspect Ratios:
Teraz w menu **Aspect Ratio** powinny być dostępne:
- **4:3** (stare rozdzielczości)
- **16:9** (1280x720, 1366x768, 1920x1080, 2560x1440)
- **16:10** (1440x900, 1680x1050, 1920x1200) 
- **21:9** (3440x1440)
- **32:9** (5120x1440)

## Jak testować v4:
1. **Aspect Ratio menu**: Sprawdź czy można przewijać między różnymi ratio
2. **Resolution menu**: Po wyborze ratio, sprawdź czy resolution się zmienia
3. **Apply**: Po wyborze np. 21:9 + 3440x1440, sprawdź czy gra przełącza się na ultrawide

## Debug v3:
Jeśli nadal nie działa, sprawdź w konsoli:
- `r_mode` - powinno być -1 dla widescreen
- `r_customwidth` - powinna być szerokość (np. 1920)
- `r_customheight` - powinna być wysokość (np. 1080)