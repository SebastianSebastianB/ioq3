#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"
#include "collision.h"

class Player {
public:
    Vec3 position;
    Vec3 velocity;
    Vec3 angles;  // pitch, yaw, roll
    
    float minZ;   // Dół bounding boxa (-24)
    float maxZ;   // Góra bounding boxa (+32)
    
    bool onGround;
    bool canWalk;
    
    Player();
    
    // Aktualizuj pozycję gracza
    void Update(float deltaTime, const TerrainData& terrain);
    
    // Obsłuż input
    void ProcessInput(bool forward, bool back, bool left, bool right, bool jump, float deltaTime);
    
    // Obróć kamerę
    void RotateView(float pitchDelta, float yawDelta);
};

#endif // PLAYER_H
