# Architektura Silnika Testowego - Szczegóły Techniczne

## Przepływ Danych

```
main.cpp (Game Loop)
    │
    ├─> Player::ProcessInput() ──> Oblicza wishVelocity na podstawie WSAD
    │
    ├─> Player::Update()
    │       │
    │       ├─> Aplikuje grawitację: velocity.z -= GRAVITY * dt
    │       │
    │       ├─> Aktualizuje pozycję: position += velocity * dt
    │       │
    │       └─> Collision::GroundTrace()
    │               │
    │               ├─> TerrainLoader::GetHeightAt() ──> Pobiera wysokość terenu
    │               │
    │               ├─> TerrainLoader::GetNormalAt()  ──> Oblicza normalną (4 próbki)
    │               │
    │               └─> Zwraca: {onGround, canWalk, groundHeight, groundNormal, tooSteep}
    │
    └─> Renderer::RenderScene()
            │
            ├─> RenderSky() ──> Pełnoekranowy quad (niebieski)
            │
            ├─> SetupCamera() ──> gluPerspective + rotate/translate
            │
            └─> RenderTerrain() ──> GL_TRIANGLE_STRIP mesh
```

## Kluczowe Algorytmy

### 1. Interpolacja Biliniowa Wysokości

```cpp
float GetHeightAt(float worldX, float worldY) {
    // Konwersja world space -> grid space
    float gridX = worldX / cellSize;
    float gridY = worldY / cellSize;
    
    // Znajdź komórkę siatki
    int x0 = floor(gridX);
    int y0 = floor(gridY);
    
    // Wagi interpolacji
    float fx = gridX - x0;  // [0..1]
    float fy = gridY - y0;  // [0..1]
    
    // Pobierz 4 narożniki
    h00 = heightMap[y0 * width + x0];
    h10 = heightMap[y0 * width + (x0+1)];
    h01 = heightMap[(y0+1) * width + x0];
    h11 = heightMap[(y0+1) * width + (x0+1)];
    
    // Interpoluj
    h0 = lerp(h00, h10, fx);
    h1 = lerp(h01, h11, fx);
    return lerp(h0, h1, fy) * verticalScale;
}
```

**Dlaczego biliniowa, a nie nearest-neighbor?**
- Płynne przejścia między komórkami
- Brak "schodków" podczas chodzenia
- Dokładniejsza kolizja

### 2. Obliczanie Normalnej Powierzchni

```cpp
Vec3 GetNormalAt(float worldX, float worldY) {
    // Próbkuj wysokość w 4 punktach (krzyż)
    delta = 1.0f;
    h_px = GetHeightAt(worldX + delta, worldY);  // +X
    h_nx = GetHeightAt(worldX - delta, worldY);  // -X
    h_py = GetHeightAt(worldX, worldY + delta);  // +Y
    h_ny = GetHeightAt(worldX, worldY - delta);  // -Y
    
    // Oblicz gradient
    normal.x = h_nx - h_px;  // Im większa różnica, tym bardziej pochylone w X
    normal.y = h_ny - h_py;  // Im większa różnica, tym bardziej pochylone w Y
    normal.z = 2.0 * delta;  // Stała (kontroluje "wagę" pionową)
    
    normalize(normal);
    return normal;
}
```

**Analiza składowej normal.z:**
- `normal.z = 1.0` → zbocze ~45°
- `normal.z = 0.7` → zbocze ~45.6° (limit MIN_WALK_NORMAL)
- `normal.z < 0.7` → zbocze za strome, nie można wejść
- `normal.z = 0.0` → ściana pionowa

### 3. Główna Funkcja Kolizji

```cpp
GroundTraceResult GroundTrace(position, minZ, velocity) {
    // 1. Pobierz wysokość terenu
    terrainHeight = GetHeightAt(position.x, position.y);
    
    // 2. Oblicz gdzie są "stopy" gracza
    playerBottom = position.z + minZ;  // minZ = -24
    
    // 3. Strefa "przyciągania" do podłoża
    TOLERANCE = 4.0f;
    
    // 4. Czy jesteśmy blisko ziemi? (i nie lecimy w górę)
    if (playerBottom < terrainHeight + TOLERANCE && velocity.z <= 0) {
        // TAK - jesteśmy blisko/na ziemi
        
        // 4a. Oblicz normalną powierzchni
        normal = GetNormalAt(position.x, position.y);
        
        // 4b. Sprawdź kąt zbocza
        if (normal.z < MIN_WALK_NORMAL) {
            // ZBOCZE ZA STROME
            return {onGround: true, canWalk: false, tooSteep: true, ...};
        } else {
            // ZBOCZE OK
            return {onGround: true, canWalk: true, tooSteep: false, ...};
        }
    } else {
        // NIE - jesteśmy w powietrzu
        return {onGround: false, canWalk: false, ...};
    }
}
```

### 4. Reakcja na Wynik Kolizji

```cpp
void Player::Update() {
    // ... grawitacja, ruch ...
    
    trace = GroundTrace(...);
    
    if (trace.onGround) {
        if (trace.canWalk) {
            // ✅ MOŻNA CHODZIĆ
            // Przyklej do terenu
            position.z = trace.groundHeight - minZ;
            
            // Zeruj prędkość pionową
            if (velocity.z < 0) velocity.z = 0;
            
        } else if (trace.tooSteep) {
            // ⚠️ ZBOCZE ZA STROME - ŚLIZGANIE
            // Usuń składową prędkości prostopadłą do zbocza
            dot = velocity · normal;
            if (dot < 0) {
                velocity = velocity - normal * dot;
            }
        }
    } else {
        // ❌ W POWIETRZU - nic nie rób, grawitacja działa
    }
}
```

## Stałe i Parametry

| Stała | Wartość | Opis |
|-------|---------|------|
| `MIN_WALK_NORMAL` | 0.7 | Minimalna normalna Z do chodzenia (~45°) |
| `PLAYER_HEIGHT` | 56.0 | Wysokość bounding boxa (-24 do +32) |
| `GROUND_STICK_TOLERANCE` | 4.0 | Strefa "przyciągania" do podłoża |
| `GRAVITY` | 800.0 | Przyspieszenie grawitacyjne |
| `MOVE_SPEED` | 320.0 | Prędkość chodzenia |
| `JUMP_VELOCITY` | 270.0 | Prędkość początkowa skoku |
| `ACCEL` | 10.0 | Przyspieszenie podczas chodzenia |
| `FRICTION` | 6.0 | Tarcie na ziemi |

## Różnice względem Quake III

### Uproszczenia w Test Silniku:
1. **Brak PM_ClipVelocity** - nie ma kolizji ze ścianami
2. **Brak PM_StepSlideMove** - nie ma "step up" na małe przeszkody
3. **Brak PM_AirMove/PM_WalkMove split** - jedna ujednolicona funkcja
4. **Uproszczone tarcie** - liniowy współczynnik, nie curve-based

### Co Jest Identyczne:
1. ✅ Algorytm `GroundTrace`
2. ✅ Obliczanie normalnej powierzchni
3. ✅ Interpolacja biliniowa wysokości
4. ✅ Detekcja stromych zboczy
5. ✅ Logika "przyklejania" do terenu

## Potencjalne Problemy i Rozwiązania

### Problem: Gracz "utyka" na nierównościach
**Przyczyna:** Brak przyklejania do terenu  
**Rozwiązanie:** `position.z = terrainHeight - minZ`

### Problem: Gracz wchodzi na strome zbocza
**Przyczyna:** `normal.z` jest źle obliczona lub `MIN_WALK_NORMAL` za niski  
**Rozwiązanie:** Debug normalne, zwiększ `MIN_WALK_NORMAL`

### Problem: Gracz ślizga się na płaskim terenie
**Przyczyna:** Zbyt duże tarcie lub źle ustawione flagi `onGround`  
**Rozwiązanie:** Sprawdź czy `onGround == true` na płaskim terenie

### Problem: Gracz "tonie" w terenie
**Przyczyna:** Brak zerowania `velocity.z` po wylądowaniu  
**Rozwiązanie:** `if (velocity.z < 0) velocity.z = 0`

## Metryki Wydajności

### Typowe Czasy Wykonania (Debug):
- `GetHeightAt()`: ~0.001ms (interpolacja)
- `GetNormalAt()`: ~0.004ms (4x GetHeightAt)
- `GroundTrace()`: ~0.005ms (GetHeightAt + GetNormalAt)
- `Player::Update()`: ~0.01ms (cały update + kolizja)
- `RenderTerrain()`: ~2-5ms (zależnie od rozmiaru mesh)

### Optymalizacje (jeśli potrzebne):
1. Cache ostatnio używanych komórek siatki
2. Frustum culling dla renderingu terenu
3. LOD (Level of Detail) dla dużych terenów
4. Spatial hash dla wielu obiektów

## Debugowanie

### Wizualizacja Normalnych
Dodaj w renderer.cpp:
```cpp
// Rysuj normalne jako małe linie
for (każdy wierzchołek) {
    Vec3 normal = GetNormalAt(x, y);
    glBegin(GL_LINES);
        glVertex3f(x, y, height);
        glVertex3f(x + normal.x * 10, y + normal.y * 10, height + normal.z * 10);
    glEnd();
}
```

### Log Pozycji Gracza
Dodaj w Player::Update():
```cpp
if (frameCounter % 60 == 0) {
    printf("Pos: (%.1f, %.1f, %.1f) Vel: (%.1f, %.1f, %.1f) OnGround: %d\n",
           position.x, position.y, position.z,
           velocity.x, velocity.y, velocity.z,
           onGround);
}
```

## Dalszy Rozwój

### Możliwe Rozszerzenia:
1. ✨ Dodanie kolizji ze sferami (drzewa, skały)
2. ✨ System water (zmiana tarcia, pływanie)
3. ✨ Teleporty
4. ✨ Moving platforms
5. ✨ Ładowanie wielu map
6. ✨ Save/load pozycji gracza

### Integracja z Quake III:
1. Skopiuj kod kolizji 1:1 do `bg_pmove.c`
2. Upewnij się że wskaźniki są ustawiane
3. Dodaj identyczne logi debug
4. Porównaj wyniki z test silnikiem
