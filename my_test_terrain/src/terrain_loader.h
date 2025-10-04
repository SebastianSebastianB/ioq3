#ifndef TERRAIN_LOADER_H
#define TERRAIN_LOADER_H

#include "types.h"
#include <string>

class TerrainLoader {
public:
    // Ładuje mapę z pliku .world
    static bool LoadTerrain(const std::string& filename, TerrainData& terrain);
    
    // Pobiera wysokość terenu w pozycji (x, y) świata z interpolacją biliniową
    static float GetHeightAt(const TerrainData& terrain, float worldX, float worldY);
    
    // Pobiera normalną powierzchni w pozycji (x, y)
    static Vec3 GetNormalAt(const TerrainData& terrain, float worldX, float worldY);
    
private:
    // Pomocnicze funkcje parsowania
    static bool ParseWorldFile(const std::string& filename, TerrainData& terrain);
};

#endif // TERRAIN_LOADER_H
