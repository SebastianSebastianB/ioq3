# Instrukcja Testowania Systemu Kolizji Terenu

## Szybki Start

1. **Kompilacja:**
   ```bash
   cd my_test_terrain
   cd build
   cmake --build . --config Debug
   ```

2. **Uruchomienie:**
   ```bash
   cd my_test_terrain
   run_test.bat
   ```

## Co Testować

### ✅ Test 1: Podstawowe Chodzenie
- [ ] Gracz spawuje się na mapie
- [ ] Gracz spada i "ląduje" na terenie (nie przechodzi przez podłoże)
- [ ] Można chodzić WSAD w normalnym tempie
- [ ] Gracz NIE "utyka" podczas chodzenia po płaskim terenie
- [ ] Gracz NIE ślizga się na płaskim terenie

**Oczekiwany wynik:** Płynne chodzenie, gracz zawsze na wysokości terenu.

### ✅ Test 2: Łagodne Zbocza (< 45°)
- [ ] Można wchodzić na łagodne wzgórza
- [ ] Nie ma "ślizgania się" w dół na łagodnych zboczach
- [ ] Gracz prawidłowo "przykleja się" do terenu podczas schodzenia

**Oczekiwany wynik:** Normalne chodzenie w górę i w dół.

### ✅ Test 3: Strome Zbocza (> 45°)
- [ ] Konsola wypisuje "ZBOCZE ZBYT STROME!" gdy próbujemy wejść na strome zbocze
- [ ] Gracz NIE MOŻE wejść na strome zbocze (jest odpychany)
- [ ] Gracz zsuwa się w dół stromego zbocza
- [ ] Podczas zsuwania widać ruch w dół zbocza

**Oczekiwany wynik:** Niemożność wspinania się, automatyczne zsuwanie.

### ✅ Test 4: Skok
- [ ] Spacja powoduje skok
- [ ] Podczas skoku gracz leci w górę, potem spada
- [ ] Po lądowaniu gracz prawidłowo wraca do stanu "na ziemi"
- [ ] Można skakać wielokrotnie

**Oczekiwany wynik:** Płynny skok z grawitacją.

### ✅ Test 5: Spadanie
- [ ] Jeśli zbiegniemy z krawędzi, gracz spada
- [ ] Podczas spadania działa grawitacja (przyspieszanie w dół)
- [ ] Po wylądowaniu gracz "przykleja się" do terenu

**Oczekiwany wynik:** Realistyczne spadanie z lądowaniem.

## Interpretacja Wyników

### ✅ Jeśli wszystko działa poprawnie w tym silniku:
Oznacza to, że:
- **Algorytm kolizji jest poprawny**
- **System "przyklejania" działa**
- **Detekcja stromych zboczy działa**

Problem w Quake III leży prawdopodobnie w:
- Integracji z systemem BSP
- Kolejności wywoływania funkcji
- Konfliktach z innymi systemami Q3

### ❌ Jeśli są problemy w tym silniku:
Oznacza to, że:
- **Sam algorytm kolizji wymaga poprawek**
- Możemy łatwo debugować w izolacji
- Możemy testować różne warianty bez kompilacji całego Q3

## Diagnostyka

### Konsola Debug
Program wypisuje na konsolę:
```
=== Terrain Collision Test ===
Ładowanie terenu z: maps/my_level3.world
Parametry terenu:
  width: XXX
  height: XXX
  cellSize: XXX
  verticalScale: XXX
Pozycja startowa gracza: (X, Y, Z)

=== STEROWANIE ===
[...]

NA ZIEMI - pos.z: XXX, terrain: XXX, normal.z: XXX
```

### Kluczowe Komunikaty
- `ZBOCZE ZBYT STROME! normal.z = X` - Detekcja stromego zbocza działa
- `NA ZIEMI - pos.z: X` - Gracz jest na podłożu
- `ŚLIZGANIE PO STROMYM ZBOCZU!` - System zsuwania aktywny
- `SKOK!` - Skok został wykonany

## Porównanie z Quake III

| Feature | Test Silnik | Quake III |
|---------|-------------|-----------|
| Heightfield terrain | ✅ | ✅ |
| BSP collision | ❌ | ✅ |
| Steep slope detection | ✅ | ✅ |
| Ground sticking | ✅ | ? |
| Player physics | ✅ (uproszczone) | ✅ (pełne) |
| Complexity | Minimalna | Bardzo wysoka |

## Następne Kroki

1. **Jeśli test silnik działa:**
   - Skopiuj dokładny kod kolizji do Q3
   - Upewnij się, że wskaźniki `pm_terrain_handle` i `pm_get_terrain_height` są ustawiane
   - Dodaj więcej logów debug w Q3 aby zobaczyć gdzie jest problem

2. **Jeśli test silnik nie działa:**
   - Eksperymenty z parametrami (`GROUND_STICK_TOLERANCE`, `MIN_WALK_NORMAL`)
   - Testowanie różnych algorytmów obliczania normalnej
   - Dodanie wizualizacji normalnych (małe strzałki na terenie)

## Znane Ograniczenia

- Brak kolizji ze ścianami/obiektami (tylko heightfield)
- Brak water/lava
- Uproszczone tarcie i przyspieszenie
- Brak zaawansowanego renderingu

To jest **celowe** - chcemy testować TYLKO kolizję z terenem.
