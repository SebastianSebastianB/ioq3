#ifndef RT_API_H
#define RT_API_H

#ifdef _WIN32
#  ifdef RT_BUILDING_DLL
#    define RT_API __declspec(dllexport)
#  else
#    define RT_API __declspec(dllimport)
#  endif
#else
#  define RT_API
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x, y, z;
} RT_Vec3;

typedef struct {
    RT_Vec3 origin;
    RT_Vec3 forward;
    RT_Vec3 right;
    RT_Vec3 up;
    float fovX;
    float fovY;
    float znear;
    float zfar;
} RT_Camera;

typedef struct {
    float xyz[3];
    float st[2];
} RT_Vert;

// User render callback: emit one quad as 4 RT_Vert (triangle strip order)
typedef void (*RT_EmitQuadFn)(const RT_Vert v[4], void* user);

typedef struct RT_Handle RT_Handle;

// Create terrain from height samples (row-major, size: width*height), each sample in meters
RT_API RT_Handle* RT_CreateFromHeights(const float* heights, int width, int height,
                                       float scaleH, float scaleV);

RT_API void RT_Destroy(RT_Handle* h);

// Per-frame update of camera
RT_API void RT_SetCamera(RT_Handle* h, const RT_Camera* cam);

// Render visible quads using internal simple LOD and frustum culling
RT_API void RT_Render(RT_Handle* h, RT_EmitQuadFn emit, void* userData);

// =============================================================================
// COLLISION API - Professional heightmap-based collision detection
// =============================================================================

// Get terrain height at world position (x, y)
// Returns height in meters, or 0.0 if out of bounds
// O(1) complexity - direct heightmap lookup with bilinear interpolation
RT_API float RT_GetHeightAt(RT_Handle* h, float worldX, float worldY);

// Trace ray from 'start' in direction 'dir' (normalized) for 'maxDist' units
// Returns 1 and fills 'hitPos' if ray hits terrain, 0 otherwise
// Uses DDA (Digital Differential Analyzer) for fast raycast
RT_API int RT_TraceRay(RT_Handle* h, const RT_Vec3* start, const RT_Vec3* dir, 
                       float maxDist, RT_Vec3* hitPos);

// Check if sphere (center + radius) intersects terrain
// Returns 1 if collision detected, 0 otherwise
// Fast rejection using grid cells, then precise heightmap check
RT_API int RT_CheckSphereCollision(RT_Handle* h, const RT_Vec3* center, 
                                   float radius, RT_Vec3* pushOut);

// Get terrain normal at world position (x, y) for surface alignment
// Calculates normal from surrounding heightmap samples
RT_API RT_Vec3 RT_GetNormalAt(RT_Handle* h, float worldX, float worldY);

#ifdef __cplusplus
}
#endif

#endif // RT_API_H
