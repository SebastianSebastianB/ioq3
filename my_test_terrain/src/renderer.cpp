#include "renderer.h"
#include <cmath>
#include <iostream>

#ifdef _WIN32
#include <GL/glu.h>
#endif

Renderer::Renderer() {
    // Inicjalizacja OpenGL
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    // Włącz basic lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    GLfloat lightPos[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    GLfloat lightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

Renderer::~Renderer() {
}

void Renderer::SetupCamera(const Player& player) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, 16.0/9.0, 1.0, 8000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Obróć kamerę
    glRotatef(-player.angles.x, 1, 0, 0);  // Pitch
    glRotatef(-player.angles.y, 0, 0, 1);  // Yaw
    
    // Przesuń na pozycję gracza (odwrotnie, bo kamera)
    glTranslatef(-player.position.x, -player.position.y, -player.position.z);
}

void Renderer::RenderSky() {
    // Wyłącz depth test i lighting dla nieba
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Renderuj pełnoekranowy quad z kolorem nieba
    glBegin(GL_QUADS);
        glColor3f(0.5f, 0.7f, 1.0f);  // Jasnoniebieski
        glVertex2f(-1, -1);
        glVertex2f( 1, -1);
        glVertex2f( 1,  1);
        glVertex2f(-1,  1);
    glEnd();
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    // Włącz z powrotem
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void Renderer::RenderTerrain(const TerrainData& terrain) {
    if (!terrain.heightMap) return;
    
    glColor3f(0.3f, 0.6f, 0.2f);  // Zielony kolor terenu
    
    // Renderuj teren jako siatkę trójkątów
    for (int y = 0; y < terrain.height - 1; y++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int x = 0; x < terrain.width; x++) {
            // Oblicz pozycje wierzchołków
            float worldX = x * terrain.cellSize;
            float worldY = y * terrain.cellSize;
            float worldY1 = (y + 1) * terrain.cellSize;
            
            int idx0 = y * terrain.width + x;
            int idx1 = (y + 1) * terrain.width + x;
            
            float h0 = terrain.heightMap[idx0] * terrain.verticalScale;
            float h1 = terrain.heightMap[idx1] * terrain.verticalScale;
            
            // Oblicz normalne (prosty aproximacja)
            Vec3 v0(worldX, worldY, h0);
            Vec3 v1(worldX, worldY1, h1);
            
            Vec3 normal0(0, 0, 1);  // Uproszczona normalna
            Vec3 normal1(0, 0, 1);
            
            if (x > 0) {
                int idxLeft0 = y * terrain.width + (x - 1);
                int idxLeft1 = (y + 1) * terrain.width + (x - 1);
                float hLeft0 = terrain.heightMap[idxLeft0] * terrain.verticalScale;
                float hLeft1 = terrain.heightMap[idxLeft1] * terrain.verticalScale;
                
                normal0.x = hLeft0 - h0;
                normal1.x = hLeft1 - h1;
            }
            
            if (y > 0) {
                int idxBack = (y - 1) * terrain.width + x;
                float hBack = terrain.heightMap[idxBack] * terrain.verticalScale;
                normal0.y = hBack - h0;
            }
            
            normal0.z = terrain.cellSize;
            normal1.z = terrain.cellSize;
            normal0.normalize();
            normal1.normalize();
            
            glNormal3f(normal0.x, normal0.y, normal0.z);
            glVertex3f(worldX, worldY, h0);
            
            glNormal3f(normal1.x, normal1.y, normal1.z);
            glVertex3f(worldX, worldY1, h1);
        }
        glEnd();
    }
}

void Renderer::RenderScene(const TerrainData& terrain, const Player& player) {
    // Wyczyść bufory
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Ustaw kolor tła (jako backup)
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    
    // Renderuj niebo
    RenderSky();
    
    // Ustaw kamerę
    SetupCamera(player);
    
    // Renderuj teren
    RenderTerrain(terrain);
    
    // Debug: renderuj punkt odniesienia w (0,0,0)
    glDisable(GL_LIGHTING);
    glPointSize(10.0f);
    glBegin(GL_POINTS);
        glColor3f(1, 0, 0);  // Czerwony
        glVertex3f(0, 0, 0);
    glEnd();
    glEnable(GL_LIGHTING);
}
