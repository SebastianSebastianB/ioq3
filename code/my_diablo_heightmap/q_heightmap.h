/*
===========================================================================
Copyright (C) 2025 - Diablo Mod Heightmap System

This file is part of the Diablo Mod heightmap collision and rendering system.
===========================================================================
*/

#ifndef Q_HEIGHTMAP_H
#define Q_HEIGHTMAP_H

#include "../qcommon/q_shared.h"

// ============================================================================
// HEIGHTMAP DATA STRUCTURES
// ============================================================================

// Konfiguracja heightmapy
typedef struct {
    int width;              // Szerokość siatki (punktów)
    int height;             // Wysokość siatki (punktów)
    float cellSize;         // Rozmiar komórki w jednostkach świata
    float verticalScale;    // Mnożnik wysokości
    vec3_t origin;          // Początek heightmapy w świecie (zwykle 0,0,0)
} heightmapConfig_t;

// Dane heightmapy
typedef struct {
    heightmapConfig_t config;
    float *heights;         // Tablica wysokości [height * width]
    qboolean loaded;        // Czy heightmapa jest załadowana
    
    // Tekstury (opcjonalne)
    char baseTexture[MAX_QPATH];
    qhandle_t baseTextureHandle;
    
    // AABB dla optymalizacji
    vec3_t mins;
    vec3_t maxs;
} heightmapData_t;

// Wynik trace dla heightmap
typedef struct {
    qboolean hitHeightmap;  // Czy trafiono w heightmap
    float fraction;         // 0.0 - 1.0, gdzie hit
    vec3_t endpos;          // Końcowa pozycja
    vec3_t plane_normal;    // Normalna płaszczyzny
    float terrainHeight;    // Wysokość terenu w punkcie kolizji
} heightmapTrace_t;

// ============================================================================
// COLLISION API (cm_heightmap.c)
// ============================================================================

// Inicjalizacja systemu heightmap
void CM_InitHeightmap(void);

// Załaduj heightmap z pliku
qboolean CM_LoadHeightmap(const char *filename, heightmapData_t *hm);

// Zwolnij pamięć heightmapy
void CM_FreeHeightmap(heightmapData_t *hm);

// Pobierz wysokość terenu w punkcie (x,y) świata
float CM_GetHeightmapHeight(const heightmapData_t *hm, float worldX, float worldY);

// Pobierz normalną terenu w punkcie (x,y) świata
void CM_GetHeightmapNormal(const heightmapData_t *hm, float worldX, float worldY, vec3_t normal);

// Wykonaj trace przeciwko heightmapie
void CM_TraceAgainstHeightmap(
    const heightmapData_t *hm,
    trace_t *trace,             // Wynik trace (modyfikowany)
    const vec3_t start,         // Punkt startowy
    const vec3_t mins,          // Bounding box mins
    const vec3_t maxs,          // Bounding box maxs
    const vec3_t end            // Punkt końcowy
);

// Sprawdź czy punkt jest nad heightmapą
qboolean CM_PointAboveHeightmap(const heightmapData_t *hm, const vec3_t point);

// ============================================================================
// RENDERING API (tr_heightmap.c)
// ============================================================================

// Inicjalizacja renderera heightmap
void TR_InitHeightmapRenderer(void);

// Zarejestruj heightmap do renderowania
void TR_RegisterHeightmap(const heightmapData_t *hm);

// Renderuj heightmap (wywoływane z renderer)
void TR_DrawHeightmap(const heightmapData_t *hm);

// Zwolnij zasoby renderera
void TR_ShutdownHeightmapRenderer(void);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Parsuj plik .world JSON
qboolean HM_ParseWorldFile(const char *filename, heightmapData_t *hm);

// Załaduj PGM heightmap
qboolean HM_LoadPGM(const char *filename, int *width, int *height, float **heights);

// Debug: wypisz info o heightmapie
void HM_PrintInfo(const heightmapData_t *hm);

// ============================================================================
// GLOBAL HEIGHTMAP INSTANCE
// ============================================================================

// Globalna instancja heightmapy dla aktualnej mapy
extern heightmapData_t g_worldHeightmap;

#endif // Q_HEIGHTMAP_H
