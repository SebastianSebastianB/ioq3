#include "collision.h"
#include "terrain_loader.h"
#include <iostream>

Collision::GroundTraceResult Collision::GroundTrace(
    const TerrainData& terrain,
    const Vec3& playerPos,
    float playerMinZ,
    const Vec3& velocity
) {
    GroundTraceResult result;
    result.onGround = false;
    result.canWalk = false;
    result.tooSteep = false;
    result.groundHeight = 0.0f;
    result.groundNormal = Vec3(0, 0, 1);
    
    // 1. Pobierz wysokość terenu bezpośrednio pod graczem
    float terrainHeight = TerrainLoader::GetHeightAt(terrain, playerPos.x, playerPos.y);
    result.groundHeight = terrainHeight;
    
    // 2. Oblicz gdzie znajdują się "stopy" gracza
    float playerBottom = playerPos.z + playerMinZ; // playerMinZ to zazwyczaj -24
    
    // 3. Strefa przyciągania do podłoża
    const float GROUND_STICK_TOLERANCE = 4.0f;
    
    // 4. Sprawdź czy gracz jest w zasięgu podłoża ORAZ czy nie porusza się w górę
    if (playerBottom < terrainHeight + GROUND_STICK_TOLERANCE && velocity.z <= 0) {
        // Jesteśmy blisko ziemi
        result.onGround = true;
        
        // 4a. Oblicz wektor normalny powierzchni terenu
        result.groundNormal = TerrainLoader::GetNormalAt(terrain, playerPos.x, playerPos.y);
        
        // 4b. Sprawdź czy zbocze jest zbyt strome do chodzenia
        if (result.groundNormal.z < MIN_WALK_NORMAL) {
            // Zbocze jest zbyt strome
            result.tooSteep = true;
            result.canWalk = false;
            
            std::cout << "ZBOCZE ZBYT STROME! normal.z = " << result.groundNormal.z 
                      << " (min: " << MIN_WALK_NORMAL << ")" << std::endl;
        } else {
            // Zbocze jest w porządku, można chodzić
            result.canWalk = true;
            result.tooSteep = false;
        }
    } else {
        // Gracz jest w powietrzu (za wysoko lub skacze w górę)
        result.onGround = false;
        result.canWalk = false;
    }
    
    return result;
}
