/*
===========================================================================
Copyright (C) 2025 - Diablo Mod Heightmap File Loader

Parsing .world JSON files and PGM heightmaps
===========================================================================
*/

#include "q_heightmap.h"
#include "../qcommon/q_shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// JSON PARSING (Simple)
// ============================================================================

static const char* FindJSONString(const char *json, const char *key, char *out, int outSize) {
    if (!json || !key || !out) return NULL;
    
    // Szukaj klucza
    char searchKey[256];
    Com_sprintf(searchKey, sizeof(searchKey), "\"%s\"", key);
    
    const char *pos = strstr(json, searchKey);
    if (!pos) return NULL;
    
    // Znajdź wartość (po :)
    pos = strchr(pos, ':');
    if (!pos) return NULL;
    pos++;
    
    // Pomiń whitespace
    while (*pos && (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r')) pos++;
    
    // Sprawdź czy to string (zaczyna się od ")
    if (*pos != '"') return NULL;
    pos++;
    
    // Kopiuj do zamykającego "
    int i = 0;
    while (*pos && *pos != '"' && i < outSize - 1) {
        out[i++] = *pos++;
    }
    out[i] = '\0';
    
    return out;
}

static float FindJSONFloat(const char *json, const char *key) {
    if (!json || !key) return 0.0f;
    
    char searchKey[256];
    Com_sprintf(searchKey, sizeof(searchKey), "\"%s\"", key);
    
    const char *pos = strstr(json, searchKey);
    if (!pos) return 0.0f;
    
    pos = strchr(pos, ':');
    if (!pos) return 0.0f;
    pos++;
    
    // Pomiń whitespace
    while (*pos && (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r')) pos++;
    
    return atof(pos);
}

// ============================================================================
// PGM LOADING
// ============================================================================

qboolean HM_LoadPGM(const char *filename, int *width, int *height, float **heights) {
    FILE *fp;
    char magic[3];
    int maxVal;
    
    Com_Printf("Loading PGM: %s\n", filename);
    
    fp = fopen(filename, "rb");
    if (!fp) {
        Com_Printf("^1ERROR: Cannot open PGM file: %s\n", filename);
        return qfalse;
    }
    
    // Czytaj magic number
    if (fscanf(fp, "%2s", magic) != 1) {
        fclose(fp);
        return qfalse;
    }
    
    if (strcmp(magic, "P2") != 0 && strcmp(magic, "P5") != 0) {
        Com_Printf("^1ERROR: Invalid PGM format (expected P2 or P5, got %s)\n", magic);
        fclose(fp);
        return qfalse;
    }
    
    // Pomiń komentarze
    int c = fgetc(fp);
    while (c == '#' || c == '\n' || c == '\r' || c == ' ' || c == '\t') {
        if (c == '#') {
            while ((c = fgetc(fp)) != '\n' && c != EOF);
        }
        c = fgetc(fp);
    }
    ungetc(c, fp);
    
    // Czytaj wymiary
    if (fscanf(fp, "%d %d", width, height) != 2) {
        fclose(fp);
        return qfalse;
    }
    
    if (fscanf(fp, "%d", &maxVal) != 1) {
        fclose(fp);
        return qfalse;
    }
    
    fgetc(fp); // Pomiń ostatni whitespace przed danymi
    
    Com_Printf("  PGM: %dx%d, maxVal=%d, format=%s\n", *width, *height, maxVal, magic);
    
    int totalPoints = (*width) * (*height);
    *heights = (float*)malloc(totalPoints * sizeof(float));
    
    if (!*heights) {
        Com_Printf("^1ERROR: Out of memory for heightmap\n");
        fclose(fp);
        return qfalse;
    }
    
    if (strcmp(magic, "P5") == 0) {
        // Binary format
        unsigned char *buffer = (unsigned char*)malloc(totalPoints);
        if (!buffer) {
            free(*heights);
            fclose(fp);
            return qfalse;
        }
        
        size_t read = fread(buffer, 1, totalPoints, fp);
        if (read != totalPoints) {
            Com_Printf("^1ERROR: PGM read error (expected %d, got %d)\n", totalPoints, (int)read);
            free(buffer);
            free(*heights);
            fclose(fp);
            return qfalse;
        }
        
        for (int i = 0; i < totalPoints; i++) {
            (*heights)[i] = (float)buffer[i] / (float)maxVal;
        }
        
        free(buffer);
    } else {
        // ASCII format
        for (int i = 0; i < totalPoints; i++) {
            int val;
            if (fscanf(fp, "%d", &val) != 1) {
                Com_Printf("^1ERROR: PGM read error at point %d\n", i);
                free(*heights);
                fclose(fp);
                return qfalse;
            }
            (*heights)[i] = (float)val / (float)maxVal;
        }
    }
    
    fclose(fp);
    Com_Printf("^2PGM loaded successfully\n");
    return qtrue;
}

// ============================================================================
// WORLD FILE PARSING
// ============================================================================

qboolean HM_ParseWorldFile(const char *filename, heightmapData_t *hm) {
    fileHandle_t f;
    int len;
    char *buffer;
    char heightmapPath[MAX_QPATH];
    char texturePath[MAX_QPATH];
    char fullHeightmapPath[MAX_QPATH];
    
    Com_Printf("Parsing world file: %s\n", filename);
    
    // Otwórz plik - używamy fopen zamiast FS_* bo jesteśmy w qcommon
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        Com_Printf("^1ERROR: Cannot open world file: %s\n", filename);
        return qfalse;
    }
    
    // Pobierz rozmiar
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if (len <= 0) {
        Com_Printf("^1ERROR: Empty world file: %s\n", filename);
        fclose(fp);
        return qfalse;
    }
    
    // Alokuj bufor
    buffer = (char*)malloc(len + 1);
    if (!buffer) {
        fclose(fp);
        return qfalse;
    }
    
    // Czytaj plik
    fread(buffer, 1, len, fp);
    buffer[len] = '\0';
    fclose(fp);
    
    // Parsuj JSON
    FindJSONString(buffer, "heightmap", heightmapPath, sizeof(heightmapPath));
    hm->config.cellSize = FindJSONFloat(buffer, "horizontal");
    hm->config.verticalScale = FindJSONFloat(buffer, "vertical");
    FindJSONString(buffer, "base", texturePath, sizeof(texturePath));
    
    free(buffer);
    
    if (heightmapPath[0] == '\0') {
        Com_Printf("^1ERROR: No heightmap path in world file\n");
        return qfalse;
    }
    
    Com_Printf("  Heightmap: %s\n", heightmapPath);
    Com_Printf("  Cell size: %.1f\n", hm->config.cellSize);
    Com_Printf("  Vertical scale: %.1f\n", hm->config.verticalScale);
    Com_Printf("  Base texture: %s\n", texturePath);
    
    // Zbuduj pełną ścieżkę do PGM (względem world file)
    // Ekstraktuj katalog z filename
    char worldDir[MAX_QPATH];
    Q_strncpyz(worldDir, filename, sizeof(worldDir));
    char *lastSlash = strrchr(worldDir, '/');
    if (!lastSlash) lastSlash = strrchr(worldDir, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0'; // Obetnij nazwę pliku, zostaw katalog
        // Wyjdź poziom wyżej (z maps/ do my_diablo_output/)
        if (lastSlash > worldDir) {
            *lastSlash = '\0';
            lastSlash = strrchr(worldDir, '/');
            if (!lastSlash) lastSlash = strrchr(worldDir, '\\');
            if (lastSlash) {
                *(lastSlash + 1) = '\0';
            }
        }
    } else {
        worldDir[0] = '\0';
    }
    
    Com_sprintf(fullHeightmapPath, sizeof(fullHeightmapPath), "%s%s", worldDir, heightmapPath);
    Com_Printf("  Full PGM path: %s\n", fullHeightmapPath);
    
    // Załaduj PGM
    if (!HM_LoadPGM(fullHeightmapPath, &hm->config.width, &hm->config.height, &hm->heights)) {
        return qfalse;
    }
    
    // Inicjalizuj origin na (0, 0, 0)
    VectorClear(hm->config.origin);
    
    // Skopiuj nazwę tekstury
    Q_strncpyz(hm->baseTexture, texturePath, sizeof(hm->baseTexture));
    
    return qtrue;
}
