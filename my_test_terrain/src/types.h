#ifndef TYPES_H
#define TYPES_H

#include <cmath>

// Podstawowe typy matematyczne
struct Vec3 {
    float x, y, z;
    
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    
    float length() const { return sqrtf(x*x + y*y + z*z); }
    void normalize() {
        float len = length();
        if (len > 0.0001f) {
            x /= len; y /= len; z /= len;
        }
    }
    
    float dot(const Vec3& v) const { return x*v.x + y*v.y + z*v.z; }
};

// Struktura mapy terenu
struct TerrainData {
    int width;           // Szerokość w werteksach
    int height;          // Wysokość w werteksach
    float cellSize;      // Rozmiar komórki w jednostkach świata
    float verticalScale; // Skala pionowa
    float* heightMap;    // Tablica wysokości [height * width]
    
    TerrainData() : width(0), height(0), cellSize(1.0f), verticalScale(1.0f), heightMap(nullptr) {}
    ~TerrainData() { if (heightMap) delete[] heightMap; }
};

// Stałe
#define MIN_WALK_NORMAL 0.7f  // Minimalna normalna dla chodzenia (~45 stopni)
#define PLAYER_HEIGHT 56.0f   // Wysokość gracza (od -24 do +32)
#define PLAYER_RADIUS 15.0f   // Promień cylindra kolizji gracza

#endif // TYPES_H
