/*
===========================================================================
Copyright (C) 2025 - Diablo Mod Heightmap Collision System

Heightmap collision detection for terrain
===========================================================================
*/

#include "q_heightmap.h"
#include "../qcommon/q_shared.h"
#include <string.h>
#include <math.h>

// Globalna heightmapa świata
heightmapData_t g_worldHeightmap;

// Stałe kolizji
#define MIN_WALK_NORMAL 0.7f        // cos(45°) - minimalna normalna dla chodzenia
#define GROUND_STICK_TOLERANCE 4.0f // Tolerancja "przyklejania" do ziemi

// ============================================================================
// INITIALIZATION
// ============================================================================

void CM_InitHeightmap(void) {
    memset(&g_worldHeightmap, 0, sizeof(g_worldHeightmap));
    g_worldHeightmap.loaded = qfalse;
    Com_Printf("Heightmap collision system initialized\n");
}

void CM_FreeHeightmap(heightmapData_t *hm) {
    if (hm->heights) {
        free(hm->heights);
        hm->heights = NULL;
    }
    hm->loaded = qfalse;
}

// ============================================================================
// HEIGHT QUERIES
// ============================================================================

float CM_GetHeightmapHeight(const heightmapData_t *hm, float worldX, float worldY) {
    if (!hm || !hm->loaded || !hm->heights) {
        return 0.0f;
    }
    
    const heightmapConfig_t *cfg = &hm->config;
    
    // Konwertuj współrzędne świata na współrzędne siatki
    float gridX = (worldX - cfg->origin[0]) / cfg->cellSize;
    float gridY = (worldY - cfg->origin[1]) / cfg->cellSize;
    
    // Znajdź komórkę siatki
    int x0 = (int)floor(gridX);
    int y0 = (int)floor(gridY);
    
    // Sprawdź granice
    if (x0 < 0 || x0 >= cfg->width - 1 || y0 < 0 || y0 >= cfg->height - 1) {
        return 0.0f; // Poza heightmapą
    }
    
    // Oblicz wagi interpolacji
    float fx = gridX - x0;
    float fy = gridY - y0;
    
    // Pobierz cztery narożniki komórki
    float h00 = hm->heights[y0 * cfg->width + x0];
    float h10 = hm->heights[y0 * cfg->width + (x0 + 1)];
    float h01 = hm->heights[(y0 + 1) * cfg->width + x0];
    float h11 = hm->heights[(y0 + 1) * cfg->width + (x0 + 1)];
    
    // Interpolacja biliniowa
    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;
    float height = h0 * (1.0f - fy) + h1 * fy;
    
    return cfg->origin[2] + height * cfg->verticalScale;
}

void CM_GetHeightmapNormal(const heightmapData_t *hm, float worldX, float worldY, vec3_t normal) {
    VectorSet(normal, 0, 0, 1); // Domyślnie w górę
    
    if (!hm || !hm->loaded || !hm->heights) {
        return;
    }
    
    // Próbkuj wysokość w czterech punktach wokół pozycji (4-point sampling)
    const float delta = 1.0f;
    float h_px = CM_GetHeightmapHeight(hm, worldX + delta, worldY);
    float h_nx = CM_GetHeightmapHeight(hm, worldX - delta, worldY);
    float h_py = CM_GetHeightmapHeight(hm, worldX, worldY + delta);
    float h_ny = CM_GetHeightmapHeight(hm, worldX, worldY - delta);
    
    // Oblicz gradient (różnice wysokości)
    normal[0] = h_nx - h_px;  // Gradient wzdłuż X
    normal[1] = h_ny - h_py;  // Gradient wzdłuż Y
    normal[2] = 2.0f * delta; // Stała składowa pionowa
    
    VectorNormalize(normal);
}

// ============================================================================
// COLLISION DETECTION
// ============================================================================

qboolean CM_PointAboveHeightmap(const heightmapData_t *hm, const vec3_t point) {
    if (!hm || !hm->loaded) {
        return qfalse;
    }
    
    // Sprawdź czy punkt jest w obszarze AABB heightmapy
    if (point[0] < hm->mins[0] || point[0] > hm->maxs[0] ||
        point[1] < hm->mins[1] || point[1] > hm->maxs[1]) {
        return qfalse;
    }
    
    return qtrue;
}

void CM_TraceAgainstHeightmap(
    const heightmapData_t *hm,
    trace_t *trace,
    const vec3_t start,
    const vec3_t mins,
    const vec3_t maxs,
    const vec3_t end
) {
    if (!hm || !hm->loaded || !trace) {
        return; // Brak heightmapy - nic nie rób
    }
    
    // Sprawdź czy trace przechodzi przez obszar heightmapy
    if (!CM_PointAboveHeightmap(hm, start) && !CM_PointAboveHeightmap(hm, end)) {
        return; // Poza heightmapą
    }
    
    // Pobierz pozycję dołu bounding boxa (gdzie stopy gracza)
    vec3_t tracePos;
    VectorCopy(start, tracePos);
    tracePos[2] += mins[2]; // Dół bounding boxa
    
    // Pobierz wysokość terenu i normalną
    float terrainZ = CM_GetHeightmapHeight(hm, tracePos[0], tracePos[1]);
    vec3_t terrainNormal;
    CM_GetHeightmapNormal(hm, tracePos[0], tracePos[1], terrainNormal);
    
    // Sprawdź czy trace przecina teren
    float groundDist = tracePos[2] - terrainZ;
    
    // Jeśli jesteśmy blisko terenu lub pod nim
    if (groundDist <= GROUND_STICK_TOLERANCE) {
        // Sprawdź czy normalna pozwala na chodzenie
        qboolean canWalk = (terrainNormal[2] >= MIN_WALK_NORMAL);
        
        // Oblicz odległość trace
        vec3_t delta;
        VectorSubtract(end, start, delta);
        float traceLength = sqrt(delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2]);
        
        if (canWalk && groundDist < trace->fraction * traceLength) {
            // Kolizja z terenem!
            trace->fraction = 0.0f; // Zatrzymaj na powierzchni
            VectorCopy(terrainNormal, trace->plane.normal);
            trace->plane.dist = terrainZ;
            trace->surfaceFlags = SURF_NODAMAGE; // Terrain nie zadaje obrażeń
            trace->contents = CONTENTS_SOLID;
            
            // Ustaw endpos na powierzchnię terenu
            VectorCopy(start, trace->endpos);
            trace->endpos[2] = terrainZ - mins[2]; // Pozycja origin gracza
            
            trace->allsolid = qfalse;
            trace->startsolid = (groundDist < 0);
            
            // Debug info
            #ifdef _DEBUG
            if (trace->startsolid) {
                Com_Printf("HEIGHTMAP: startsolid at z=%.1f, terrain=%.1f\n", 
                          tracePos[2], terrainZ);
            }
            #endif
        }
    }
}

// ============================================================================
// FILE LOADING
// ============================================================================

qboolean CM_LoadHeightmap(const char *filename, heightmapData_t *hm) {
    if (!filename || !hm) {
        return qfalse;
    }
    
    Com_Printf("CM_LoadHeightmap: %s\n", filename);
    
    // Wyczyść poprzednie dane
    CM_FreeHeightmap(hm);
    
    // Parsuj plik .world
    if (!HM_ParseWorldFile(filename, hm)) {
        Com_Printf("^1ERROR: Failed to parse world file: %s\n", filename);
        return qfalse;
    }
    
    // Oblicz AABB
    VectorSet(hm->mins, 
              hm->config.origin[0], 
              hm->config.origin[1], 
              hm->config.origin[2]);
    
    VectorSet(hm->maxs,
              hm->config.origin[0] + hm->config.width * hm->config.cellSize,
              hm->config.origin[1] + hm->config.height * hm->config.cellSize,
              hm->config.origin[2] + hm->config.verticalScale);
    
    hm->loaded = qtrue;
    
    Com_Printf("^2Heightmap loaded successfully: %dx%d\n", 
               hm->config.width, hm->config.height);
    HM_PrintInfo(hm);
    
    return qtrue;
}

// ============================================================================
// DEBUG
// ============================================================================

void HM_PrintInfo(const heightmapData_t *hm) {
    if (!hm || !hm->loaded) {
        Com_Printf("Heightmap: NOT LOADED\n");
        return;
    }
    
    Com_Printf("=== HEIGHTMAP INFO ===\n");
    Com_Printf("  Size: %dx%d\n", hm->config.width, hm->config.height);
    Com_Printf("  Cell: %.1f units\n", hm->config.cellSize);
    Com_Printf("  V.Scale: %.1f\n", hm->config.verticalScale);
    Com_Printf("  Origin: (%.1f, %.1f, %.1f)\n", 
               hm->config.origin[0], hm->config.origin[1], hm->config.origin[2]);
    Com_Printf("  AABB: (%.1f,%.1f,%.1f) - (%.1f,%.1f,%.1f)\n",
               hm->mins[0], hm->mins[1], hm->mins[2],
               hm->maxs[0], hm->maxs[1], hm->maxs[2]);
    Com_Printf("  Texture: %s\n", hm->baseTexture[0] ? hm->baseTexture : "none");
    Com_Printf("======================\n");
}
