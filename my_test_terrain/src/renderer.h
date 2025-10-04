#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"
#include "player.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    // Renderuj scenę
    void RenderScene(const TerrainData& terrain, const Player& player);
    
private:
    // Renderuj teren
    void RenderTerrain(const TerrainData& terrain);
    
    // Renderuj niebo
    void RenderSky();
    
    // Ustaw kamerę na podstawie pozycji i kątów gracza
    void SetupCamera(const Player& player);
};

#endif // RENDERER_H
