#ifndef COLLISION_H
#define COLLISION_H

#include "types.h"

class Collision {
public:
    // Testuje kolizję gracza z terenem i zwraca informacje o kontakcie
    struct GroundTraceResult {
        bool onGround;          // Czy gracz jest na ziemi
        bool canWalk;           // Czy może chodzić (zbocze nie za strome)
        float groundHeight;     // Wysokość podłoża
        Vec3 groundNormal;      // Normalna powierzchni
        bool tooSteep;          // Czy zbocze jest za strome
    };
    
    // Główna funkcja kolizji - odpowiednik PM_GroundTrace
    static GroundTraceResult GroundTrace(
        const TerrainData& terrain,
        const Vec3& playerPos,
        float playerMinZ,
        const Vec3& velocity
    );
};

#endif // COLLISION_H
