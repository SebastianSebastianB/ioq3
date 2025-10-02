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

#ifdef __cplusplus
}
#endif

#endif // RT_API_H
