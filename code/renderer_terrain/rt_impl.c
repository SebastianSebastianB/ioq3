#define RT_BUILDING_DLL
#include "rt_api.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct RT_Handle {
    int width, height;
    float scaleH, scaleV;
    float* h; // heights in meters
    RT_Camera cam;
    float uvScale; // Texture tiling scale (default 1.0, higher = more repeats)
};

static float clampf(float v, float a, float b){ return v<a?a:(v>b?b:v);} 
static float dot3(RT_Vec3 a, RT_Vec3 b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
static RT_Vec3 add3(RT_Vec3 a, RT_Vec3 b){ RT_Vec3 r={a.x+b.x,a.y+b.y,a.z+b.z}; return r; }
static RT_Vec3 mul3(RT_Vec3 a, float s){ RT_Vec3 r={a.x*s,a.y*s,a.z*s}; return r; }

RT_Handle* RT_CreateFromHeights(const float* heights, int width, int height,
                                float scaleH, float scaleV){
    if(width<2||height<2||!heights) return NULL;
    RT_Handle* h = (RT_Handle*)calloc(1, sizeof(*h));
    if(!h) return NULL;
    h->width=width; h->height=height; h->scaleH=scaleH; h->scaleV=scaleV;
    h->uvScale = 4.0f; // Default tiling: 4x texture repeat (adjust as needed)
    size_t n = (size_t)width*height;
    h->h = (float*)malloc(n*sizeof(float));
    if(!h->h){ free(h); return NULL; }
    memcpy(h->h, heights, n*sizeof(float));
    return h;
}

void RT_Destroy(RT_Handle* h){
    if(!h) return;
    free(h->h);
    free(h);
}

void RT_SetCamera(RT_Handle* h, const RT_Camera* cam){ if(h&&cam) h->cam=*cam; }

typedef struct { float minx,miny,minz,maxx,maxy,maxz; } AABB;
static AABB tileBounds(RT_Handle* t, int x0,int y0,int x1,int y1){
    AABB b={1e9f,1e9f,1e9f,-1e9f,-1e9f,-1e9f};
    for(int y=y0;y<=y1;y++){
        for(int x=x0;x<=x1;x++){
            float wx = x * t->scaleH;
            float wy = y * t->scaleH;
            float wz = t->h[y*t->width+x] * t->scaleV;
            if(wx<b.minx) b.minx=wx; if(wx>b.maxx) b.maxx=wx;
            if(wy<b.miny) b.miny=wy; if(wy>b.maxy) b.maxy=wy;
            if(wz<b.minz) b.minz=wz; if(wz>b.maxz) b.maxz=wz;
        }
    }
    return b;
}

static int aabbVisible(const AABB* b, const RT_Camera* c){
    // Very conservative visibility: distance and FOV cone with margin for edge triangles
    RT_Vec3 center = { (b->minx+b->maxx)*0.5f, (b->miny+b->maxy)*0.5f, (b->minz+b->maxz)*0.5f };
    RT_Vec3 v = { center.x - c->origin.x, center.y - c->origin.y, center.z - c->origin.z };
    float dist2 = dot3(v,v);
    if(dist2> c->zfar*c->zfar) return 0;
    float d = dot3(v, c->forward);
    
    // Allow tiles slightly behind camera (helps with edge cases)
    float tileRadius = sqrtf((b->maxx-b->minx)*(b->maxx-b->minx) + 
                             (b->maxy-b->miny)*(b->maxy-b->miny) + 
                             (b->maxz-b->minz)*(b->maxz-b->minz)) * 0.5f;
    if(d < -tileRadius) return 0;
    
    // Relaxed FOV check with margin (1.5x wider to catch edge triangles)
    float halfFovX = c->fovX*0.5f * (3.14159265f/180.f) * 1.5f;
    float halfFovY = c->fovY*0.5f * (3.14159265f/180.f) * 1.5f;
    // Project v to camera basis
    float vx = dot3(v, c->right);
    float vy = dot3(v, c->up);
    float vz = dot3(v, c->forward);
    
    // Avoid division by zero
    if(fabsf(vz) < 0.01f) return 1;
    
    float ax = fabsf(vx/vz);
    float ay = fabsf(vy/vz);
    if(ax>tanf(halfFovX) || ay>tanf(halfFovY)) return 0;
    return 1;
}

static void emitQuadRT(RT_Handle* t, int x, int y, int step, RT_EmitQuadFn emit, void* user){
    int x1 = x+step, y1=y+step;
    if(x1>=t->width||y1>=t->height) return;
    RT_Vert v[4];
    int idx;
    // CW winding order (reversed from CCW)
    // Bottom-left (0,0)
    idx = y*t->width + x;    
    v[0].xyz[0]=x*t->scaleH; 
    v[0].xyz[1]=y*t->scaleH; 
    v[0].xyz[2]=t->h[idx]*t->scaleV; 
    v[0].st[0]=(float)x/(float)(t->width-1) * t->uvScale; // Apply UV tiling scale
    v[0].st[1]=(float)y/(float)(t->height-1) * t->uvScale;
    
    // Top-left (0,1)
    idx = y1*t->width + x;   
    v[1].xyz[0]=x*t->scaleH; 
    v[1].xyz[1]=y1*t->scaleH;
    v[1].xyz[2]=t->h[idx]*t->scaleV; 
    v[1].st[0]=(float)x/(float)(t->width-1) * t->uvScale;
    v[1].st[1]=(float)y1/(float)(t->height-1) * t->uvScale;
    
    // Top-right (1,1)
    idx = y1*t->width + x1;  
    v[2].xyz[0]=x1*t->scaleH;
    v[2].xyz[1]=y1*t->scaleH;
    v[2].xyz[2]=t->h[idx]*t->scaleV; 
    v[2].st[0]=(float)x1/(float)(t->width-1) * t->uvScale;
    v[2].st[1]=(float)y1/(float)(t->height-1) * t->uvScale;
    
    // Bottom-right (1,0)
    idx = y*t->width + x1;   
    v[3].xyz[0]=x1*t->scaleH;
    v[3].xyz[1]=y*t->scaleH; 
    v[3].xyz[2]=t->h[idx]*t->scaleV; 
    v[3].st[0]=(float)x1/(float)(t->width-1) * t->uvScale;
    v[3].st[1]=(float)y/(float)(t->height-1) * t->uvScale;
    
    emit(v, user);
}

void RT_Render(RT_Handle* t, RT_EmitQuadFn emit, void* user){
    if(!t||!emit) return;
    // Simple tile-based LOD: tile size grows with distance
    int baseStep = 1;
    int quadCount = 0;
    int maxQuads = 4096; // Safety limit to prevent overflow
    
    for(int ty=0; ty<t->height-1; ){
        for(int tx=0; tx<t->width-1; ){
            // Check quad limit to prevent crashes
            if(quadCount >= maxQuads) {
                return; // Stop rendering to prevent overflow
            }
            
            // Determine step based on distance to tile center
            AABB b = tileBounds(t, tx, ty, tx+1, ty+1);
            if(!aabbVisible(&b, &t->cam)) { tx+=1; continue; }
            RT_Vec3 c = { (b.minx+b.maxx)*0.5f, (b.miny+b.maxy)*0.5f, (b.minz+b.maxz)*0.5f };
            RT_Vec3 v = { c.x - t->cam.origin.x, c.y - t->cam.origin.y, c.z - t->cam.origin.z };
            float d = sqrtf(dot3(v,v));
            int step = baseStep;
            // More aggressive LOD to reduce poly count
            if(d>2048) step=16; 
            else if(d>1024) step=8; 
            else if(d>512) step=4; 
            else if(d>256) step=2; 
            else step=1;
            // Clamp step to not overshoot
            if(tx+step>=t->width) step = t->width-1 - tx;
            if(ty+step>=t->height) step = (step < (t->height-1-ty)) ? step : (t->height-1-ty);
            // Frustum cull the tile AABB
            AABB tb = tileBounds(t, tx, ty, tx+step, ty+step);
            if(aabbVisible(&tb, &t->cam)){
                // Emit coarse quad; in practice, you would tessellate within the tile
                emitQuadRT(t, tx, ty, step, emit, user);
                quadCount++;
            }
            tx += step;
        }
        ty += 1;
    }
}

// =============================================================================
// COLLISION IMPLEMENTATION - Professional heightmap-based collision
// =============================================================================

// Helper: Clamp grid coordinates to valid range
static void clampGridCoords(RT_Handle* h, int* gx, int* gy) {
    if(*gx < 0) *gx = 0;
    if(*gy < 0) *gy = 0;
    if(*gx >= h->width) *gx = h->width - 1;
    if(*gy >= h->height) *gy = h->height - 1;
}

// Helper: World to grid coordinates
static void worldToGrid(RT_Handle* h, float wx, float wy, int* gx, int* gy) {
    *gx = (int)(wx / h->scaleH);
    *gy = (int)(wy / h->scaleH);
    clampGridCoords(h, gx, gy);
}

// Get terrain height at world position with bilinear interpolation
float RT_GetHeightAt(RT_Handle* h, float worldX, float worldY) {
    if(!h) return 0.0f;
    
    // Convert world coords to grid space (continuous)
    float gx = worldX / h->scaleH;
    float gy = worldY / h->scaleH;
    
    // Get integer grid coordinates
    int x0 = (int)floorf(gx);
    int y0 = (int)floorf(gy);
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    
    // Clamp to valid range
    if(x0 < 0) x0 = 0;
    if(y0 < 0) y0 = 0;
    if(x1 >= h->width) x1 = h->width - 1;
    if(y1 >= h->height) y1 = h->height - 1;
    if(x0 >= h->width) x0 = h->width - 1;
    if(y0 >= h->height) y0 = h->height - 1;
    
    // Get fractional part for interpolation
    float fx = gx - floorf(gx);
    float fy = gy - floorf(gy);
    
    // Sample 4 heightmap corners
    float h00 = h->h[y0 * h->width + x0] * h->scaleV;
    float h10 = h->h[y0 * h->width + x1] * h->scaleV;
    float h01 = h->h[y1 * h->width + x0] * h->scaleV;
    float h11 = h->h[y1 * h->width + x1] * h->scaleV;
    
    // Bilinear interpolation
    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;
    float height = h0 * (1.0f - fy) + h1 * fy;
    
    return height;
}

// ✨ NOWE: Get vertical scale for dynamic tolerance calculation
float RT_GetVerticalScale(RT_Handle* h) {
    if(!h) return 0.0f;
    return h->scaleV;
}

// Get terrain normal at world position (for surface alignment)
RT_Vec3 RT_GetNormalAt(RT_Handle* h, float worldX, float worldY) {
    RT_Vec3 normal = {0.0f, 0.0f, 1.0f}; // Default up
    if(!h) return normal;
    
    // Sample height at center and neighbors (using small offset)
    float offset = h->scaleH; // One grid cell
    float hC = RT_GetHeightAt(h, worldX, worldY);
    float hL = RT_GetHeightAt(h, worldX - offset, worldY);
    float hR = RT_GetHeightAt(h, worldX + offset, worldY);
    float hD = RT_GetHeightAt(h, worldX, worldY - offset);
    float hU = RT_GetHeightAt(h, worldX, worldY + offset);
    
    // Calculate tangent vectors
    RT_Vec3 tangentX = {2.0f * offset, 0.0f, hR - hL};
    RT_Vec3 tangentY = {0.0f, 2.0f * offset, hU - hD};
    
    // Cross product for normal
    normal.x = tangentX.y * tangentY.z - tangentX.z * tangentY.y;
    normal.y = tangentX.z * tangentY.x - tangentX.x * tangentY.z;
    normal.z = tangentX.x * tangentY.y - tangentX.y * tangentY.x;
    
    // Normalize
    float len = sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
    if(len > 0.001f) {
        normal.x /= len;
        normal.y /= len;
        normal.z /= len;
    }
    
    return normal;
}

// Check sphere collision with terrain (for player/object collision)
int RT_CheckSphereCollision(RT_Handle* h, const RT_Vec3* center, float radius, RT_Vec3* pushOut) {
    if(!h || !center) return 0;
    
    // Check height at sphere center
    float terrainHeight = RT_GetHeightAt(h, center->x, center->y);
    float sphereBottom = center->z - radius;
    
    // Allow small tolerance for stability (0.5 units above terrain)
    float tolerance = 0.5f;
    
    // If sphere bottom is above terrain (with tolerance), no collision
    if(sphereBottom > terrainHeight + tolerance) {
        if(pushOut) {
            pushOut->x = 0.0f;
            pushOut->y = 0.0f;
            pushOut->z = 0.0f;
        }
        return 0;
    }
    
    // Collision detected - calculate push-out vector
    if(pushOut) {
        float penetration = terrainHeight - sphereBottom;
        
        // Add small upward bias to ensure player stays above terrain
        penetration += 1.0f; // Push up by 1 extra unit for stability
        
        RT_Vec3 normal = RT_GetNormalAt(h, center->x, center->y);
        pushOut->x = normal.x * penetration;
        pushOut->y = normal.y * penetration;
        pushOut->z = normal.z * penetration;
    }
    
    return 1;
}

// Ray-terrain intersection using DDA (Digital Differential Analyzer)
int RT_TraceRay(RT_Handle* h, const RT_Vec3* start, const RT_Vec3* dir, float maxDist, RT_Vec3* hitPos) {
    if(!h || !start || !dir) return 0;
    
    // DDA parameters
    float stepSize = h->scaleH * 0.5f; // Half grid cell for accuracy
    int maxSteps = (int)(maxDist / stepSize) + 1;
    
    RT_Vec3 pos = *start;
    RT_Vec3 step = {dir->x * stepSize, dir->y * stepSize, dir->z * stepSize};
    
    for(int i = 0; i < maxSteps; i++) {
        // Get terrain height at current ray position
        float terrainHeight = RT_GetHeightAt(h, pos.x, pos.y);
        
        // Check if ray went below terrain
        if(pos.z <= terrainHeight) {
            if(hitPos) {
                // Refine hit position (simple linear interpolation)
                *hitPos = pos;
                hitPos->z = terrainHeight;
            }
            return 1;
        }
        
        // Advance ray
        pos.x += step.x;
        pos.y += step.y;
        pos.z += step.z;
        
        // Check if ray left terrain bounds
        if(pos.x < 0 || pos.y < 0 || 
           pos.x >= (h->width-1) * h->scaleH || 
           pos.y >= (h->height-1) * h->scaleH) {
            break;
        }
    }
    
    return 0; // No hit
}
