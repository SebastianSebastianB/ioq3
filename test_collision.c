// ==============================================================================
// TERRAIN COLLISION TEST PROGRAM
// ==============================================================================
// Compile: gcc -o test_collision test_collision.c -I../renderer_terrain -L../build/Debug/my_diablo_output -lrenderer_terrain -lm
// Run: ./test_collision

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../code/renderer_terrain/rt_api.h"

// Helper: Generate simple test heightmap (sinusoidal terrain)
float* GenerateTestHeightmap(int width, int height) {
    float* heights = (float*)malloc(width * height * sizeof(float));
    
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            // Sinusoidal terrain: amplitude 50, frequency 0.1
            float h = 50.0f * sinf(x * 0.1f) * cosf(y * 0.1f);
            heights[y * width + x] = h;
        }
    }
    
    return heights;
}

// Test 1: Height Query
void Test_HeightQuery(RT_Handle* terrain) {
    printf("\n========== TEST 1: Height Query ==========\n");
    
    // Test corners
    struct { float x, y; const char* name; } testPoints[] = {
        {0.0f, 0.0f, "Origin (0,0)"},
        {100.0f, 100.0f, "Center"},
        {199.0f, 199.0f, "Far corner"},
        {50.5f, 75.3f, "Interpolated position"}
    };
    
    for(int i = 0; i < 4; i++) {
        float h = RT_GetHeightAt(terrain, testPoints[i].x, testPoints[i].y);
        printf("  %s: world(%.1f, %.1f) → height = %.2f\n", 
               testPoints[i].name, testPoints[i].x, testPoints[i].y, h);
    }
    
    printf("✓ Height query test passed\n");
}

// Test 2: Normal Calculation
void Test_NormalCalculation(RT_Handle* terrain) {
    printf("\n========== TEST 2: Normal Calculation ==========\n");
    
    float testX = 100.0f, testY = 100.0f;
    RT_Vec3 normal = RT_GetNormalAt(terrain, testX, testY);
    
    printf("  Position: (%.1f, %.1f)\n", testX, testY);
    printf("  Normal: (%.3f, %.3f, %.3f)\n", normal.x, normal.y, normal.z);
    
    // Check if normal is normalized (length ≈ 1.0)
    float len = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
    printf("  Normal length: %.6f (should be ~1.0)\n", len);
    
    if(fabs(len - 1.0f) < 0.01f) {
        printf("✓ Normal calculation test passed\n");
    } else {
        printf("✗ WARNING: Normal not normalized!\n");
    }
}

// Test 3: Sphere Collision
void Test_SphereCollision(RT_Handle* terrain) {
    printf("\n========== TEST 3: Sphere Collision ==========\n");
    
    // Test sphere on ground
    RT_Vec3 center1 = {100.0f, 100.0f, 0.0f}; // Below terrain
    float radius = 10.0f;
    RT_Vec3 pushOut;
    
    int hit1 = RT_CheckSphereCollision(terrain, &center1, radius, &pushOut);
    printf("  Test 1: Sphere at ground level\n");
    printf("    Center: (%.1f, %.1f, %.1f), Radius: %.1f\n", 
           center1.x, center1.y, center1.z, radius);
    printf("    Collision: %s\n", hit1 ? "YES" : "NO");
    if(hit1) {
        printf("    Push-out: (%.2f, %.2f, %.2f)\n", pushOut.x, pushOut.y, pushOut.z);
    }
    
    // Test sphere high above terrain
    RT_Vec3 center2 = {100.0f, 100.0f, 200.0f}; // High above
    int hit2 = RT_CheckSphereCollision(terrain, &center2, radius, &pushOut);
    printf("\n  Test 2: Sphere high above terrain\n");
    printf("    Center: (%.1f, %.1f, %.1f), Radius: %.1f\n", 
           center2.x, center2.y, center2.z, radius);
    printf("    Collision: %s\n", hit2 ? "YES" : "NO");
    
    if(hit1 && !hit2) {
        printf("✓ Sphere collision test passed\n");
    } else {
        printf("✗ Sphere collision test FAILED\n");
    }
}

// Test 4: Ray Tracing
void Test_RayTracing(RT_Handle* terrain) {
    printf("\n========== TEST 4: Ray Tracing ==========\n");
    
    // Ray from above pointing down
    RT_Vec3 start = {100.0f, 100.0f, 200.0f};
    RT_Vec3 dir = {0.0f, 0.0f, -1.0f}; // Straight down
    float maxDist = 300.0f;
    RT_Vec3 hitPos;
    
    int hit = RT_TraceRay(terrain, &start, &dir, maxDist, &hitPos);
    
    printf("  Ray: start=(%.1f, %.1f, %.1f), dir=(%.1f, %.1f, %.1f)\n",
           start.x, start.y, start.z, dir.x, dir.y, dir.z);
    printf("  Max distance: %.1f\n", maxDist);
    printf("  Hit: %s\n", hit ? "YES" : "NO");
    
    if(hit) {
        printf("  Hit position: (%.2f, %.2f, %.2f)\n", hitPos.x, hitPos.y, hitPos.z);
        
        // Verify hit position is at terrain height
        float terrainHeight = RT_GetHeightAt(terrain, hitPos.x, hitPos.y);
        printf("  Terrain height at hit: %.2f\n", terrainHeight);
        printf("  Difference: %.4f (should be ~0)\n", fabs(hitPos.z - terrainHeight));
        
        if(fabs(hitPos.z - terrainHeight) < 1.0f) {
            printf("✓ Ray tracing test passed\n");
        } else {
            printf("✗ Ray tracing accuracy warning\n");
        }
    } else {
        printf("✗ Ray tracing test FAILED (no hit detected)\n");
    }
}

// Main test program
int main() {
    printf("==============================================\n");
    printf("  TERRAIN COLLISION SYSTEM TEST\n");
    printf("==============================================\n");
    
    // Create test terrain (200x200 heightmap, 1m scale)
    int width = 200, height = 200;
    float* heights = GenerateTestHeightmap(width, height);
    
    RT_Handle* terrain = RT_CreateFromHeights(heights, width, height, 1.0f, 1.0f);
    free(heights);
    
    if(!terrain) {
        printf("ERROR: Failed to create terrain!\n");
        return 1;
    }
    
    printf("Terrain created: %dx%d, scale H=1.0, V=1.0\n", width, height);
    
    // Run tests
    Test_HeightQuery(terrain);
    Test_NormalCalculation(terrain);
    Test_SphereCollision(terrain);
    Test_RayTracing(terrain);
    
    // Cleanup
    RT_Destroy(terrain);
    
    printf("\n==============================================\n");
    printf("  ALL TESTS COMPLETED\n");
    printf("==============================================\n");
    
    return 0;
}
