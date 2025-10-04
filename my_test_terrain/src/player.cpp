#include "player.h"
#include "terrain_loader.h"
#include <iostream>
#include <cmath>

Player::Player() {
    position = Vec3(0, 0, 100);  // Start w środku mapy, 100 jednostek nad ziemią
    velocity = Vec3(0, 0, 0);
    angles = Vec3(0, 0, 0);
    
    minZ = -24.0f;  // Dół bounding boxa
    maxZ = 32.0f;   // Góra bounding boxa
    
    onGround = false;
    canWalk = false;
}

void Player::Update(float deltaTime, const TerrainData& terrain) {
    // Grawitacja
    const float GRAVITY = 800.0f;
    if (!onGround) {
        velocity.z -= GRAVITY * deltaTime;
    }
    
    // Zastosuj prędkość do pozycji
    position += velocity * deltaTime;
    
    // === KOLIZJA Z TERENEM ===
    Collision::GroundTraceResult trace = Collision::GroundTrace(
        terrain, position, minZ, velocity
    );
    
    onGround = trace.onGround;
    canWalk = trace.canWalk;
    
    if (trace.onGround) {
        if (trace.canWalk) {
            // Można chodzić - przyklej gracza do terenu
            position.z = trace.groundHeight - minZ;
            
            // Zeruj pionową prędkość
            if (velocity.z < 0) {
                velocity.z = 0;
            }
            
            // Debug info
            static int debugCounter = 0;
            if (debugCounter++ % 60 == 0) {
                std::cout << "NA ZIEMI - pos.z: " << position.z 
                          << ", terrain: " << trace.groundHeight 
                          << ", normal.z: " << trace.groundNormal.z << std::endl;
            }
        } else if (trace.tooSteep) {
            // Zbocze za strome - ślizganie się
            // Usuń składową prędkości prostopadłą do zbocza
            float dot = velocity.dot(trace.groundNormal);
            if (dot < 0) {
                velocity = velocity - trace.groundNormal * dot;
            }
            
            std::cout << "ŚLIZGANIE PO STROMYM ZBOCZU!" << std::endl;
        }
    }
    
    // Ograniczenie prędkości opadania (terminal velocity)
    if (velocity.z < -2000.0f) {
        velocity.z = -2000.0f;
    }
}

void Player::ProcessInput(bool forward, bool back, bool left, bool right, bool jump, float deltaTime) {
    const float MOVE_SPEED = 320.0f;      // Jednostek na sekundę
    const float ACCEL = 10.0f;            // Przyspieszenie
    const float FRICTION = 6.0f;          // Tarcie
    const float JUMP_VELOCITY = 270.0f;   // Prędkość skoku
    
    // Oblicz kierunek ruchu w oparciu o kąty kamery
    float yawRad = angles.y * 3.14159f / 180.0f;
    Vec3 forward_dir(-sinf(yawRad), cosf(yawRad), 0);
    Vec3 right_dir(cosf(yawRad), sinf(yawRad), 0);
    
    // Kierunek ruchu
    Vec3 moveDir(0, 0, 0);
    if (forward) moveDir += forward_dir;
    if (back) moveDir = moveDir - forward_dir;
    if (left) moveDir = moveDir - right_dir;
    if (right) moveDir += right_dir;
    
    // Normalizuj kierunek
    float len = moveDir.length();
    if (len > 0.001f) {
        moveDir = moveDir * (1.0f / len);
    }
    
    // Zastosuj przyspieszenie (tylko na ziemi)
    if (onGround && canWalk) {
        Vec3 wishVel = moveDir * MOVE_SPEED;
        Vec3 currentVelXY(velocity.x, velocity.y, 0);
        Vec3 addVel = wishVel - currentVelXY;
        
        float addSpeed = addVel.length();
        if (addSpeed > 0.001f) {
            addVel = addVel * (1.0f / addSpeed);
            float accelSpeed = ACCEL * MOVE_SPEED * deltaTime;
            if (accelSpeed > addSpeed) accelSpeed = addSpeed;
            
            velocity.x += addVel.x * accelSpeed;
            velocity.y += addVel.y * accelSpeed;
        }
        
        // Tarcie
        float speed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y);
        if (speed > 0.001f) {
            float control = (speed < MOVE_SPEED / 3.0f) ? (MOVE_SPEED / 3.0f) : speed;
            float drop = control * FRICTION * deltaTime;
            float newSpeed = speed - drop;
            if (newSpeed < 0) newSpeed = 0;
            
            float scale = newSpeed / speed;
            velocity.x *= scale;
            velocity.y *= scale;
        }
    }
    
    // Skok
    if (jump && onGround && canWalk) {
        velocity.z = JUMP_VELOCITY;
        onGround = false;
        std::cout << "SKOK!" << std::endl;
    }
}

void Player::RotateView(float pitchDelta, float yawDelta) {
    angles.x += pitchDelta;
    angles.y += yawDelta;
    
    // Ograniczenie pitch (nie możemy patrzeć całkowicie w górę/dół)
    if (angles.x > 89.0f) angles.x = 89.0f;
    if (angles.x < -89.0f) angles.x = -89.0f;
    
    // Normalizuj yaw do zakresu 0-360
    while (angles.y < 0) angles.y += 360.0f;
    while (angles.y >= 360.0f) angles.y -= 360.0f;
}
