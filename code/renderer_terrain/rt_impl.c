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
    v[0].st[0]=(float)x/(float)(t->width-1); 
    v[0].st[1]=(float)y/(float)(t->height-1);
    
    // Top-left (0,1)
    idx = y1*t->width + x;   
    v[1].xyz[0]=x*t->scaleH; 
    v[1].xyz[1]=y1*t->scaleH;
    v[1].xyz[2]=t->h[idx]*t->scaleV; 
    v[1].st[0]=(float)x/(float)(t->width-1); 
    v[1].st[1]=(float)y1/(float)(t->height-1);
    
    // Top-right (1,1)
    idx = y1*t->width + x1;  
    v[2].xyz[0]=x1*t->scaleH;
    v[2].xyz[1]=y1*t->scaleH;
    v[2].xyz[2]=t->h[idx]*t->scaleV; 
    v[2].st[0]=(float)x1/(float)(t->width-1); 
    v[2].st[1]=(float)y1/(float)(t->height-1);
    
    // Bottom-right (1,0)
    idx = y*t->width + x1;   
    v[3].xyz[0]=x1*t->scaleH;
    v[3].xyz[1]=y*t->scaleH; 
    v[3].xyz[2]=t->h[idx]*t->scaleV; 
    v[3].st[0]=(float)x1/(float)(t->width-1); 
    v[3].st[1]=(float)y/(float)(t->height-1);
    
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
