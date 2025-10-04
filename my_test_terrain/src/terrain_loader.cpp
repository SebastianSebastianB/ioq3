#include "terrain_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <windows.h>
#include <GL/gl.h>

#pragma pack(push, 1)
struct TGAHeader {
    unsigned char idLength;
    unsigned char colorMapType;
    unsigned char imageType;
    unsigned short colorMapOrigin;
    unsigned short colorMapLength;
    unsigned char colorMapDepth;
    unsigned short xOrigin;
    unsigned short yOrigin;
    unsigned short width;
    unsigned short height;
    unsigned char pixelDepth;
    unsigned char imageDescriptor;
};
#pragma pack(pop)

// Pomocnicza funkcja do ładowania PGM heightmap
static bool LoadPGM(const std::string& filename, int& width, int& height, float*& heightMap) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć PGM: " << filename << std::endl;
        return false;
    }
    
    std::string magic;
    file >> magic;
    
    if (magic != "P2" && magic != "P5") {
        std::cerr << "Nieprawidłowy format PGM (oczekiwano P2 lub P5, otrzymano: " << magic << ")" << std::endl;
        return false;
    }
    
    // Pomiń komentarze
    char c = file.get();
    while (c == '#' || c == '\n' || c == '\r' || c == ' ' || c == '\t') {
        if (c == '#') {
            std::string comment;
            std::getline(file, comment);
        }
        c = file.get();
    }
    file.unget();
    
    // Czytaj wymiary
    file >> width >> height;
    
    int maxVal;
    file >> maxVal;
    
    // Pomiń whitespace przed danymi
    file.get();
    
    std::cout << "Ładowanie PGM: " << width << "x" << height << ", maxVal=" << maxVal << std::endl;
    
    int totalPoints = width * height;
    heightMap = new float[totalPoints];
    
    if (magic == "P5") {
        // Binary format
        unsigned char* buffer = new unsigned char[totalPoints];
        file.read((char*)buffer, totalPoints);
        
        for (int i = 0; i < totalPoints; i++) {
            heightMap[i] = (float)buffer[i] / (float)maxVal;
        }
        
        delete[] buffer;
    } else {
        // ASCII format
        for (int i = 0; i < totalPoints; i++) {
            int val;
            if (!(file >> val)) {
                std::cerr << "Błąd odczytu PGM w punkcie " << i << std::endl;
                delete[] heightMap;
                return false;
            }
            heightMap[i] = (float)val / (float)maxVal;
        }
    }
    
    std::cout << "Pomyślnie załadowano PGM heightmap" << std::endl;
    return true;
}

// Pomocnicza funkcja do parsowania JSON (bardzo prosty parser)
static std::string GetJSONStringValue(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    
    pos = json.find("\"", pos + key.length() + 2);
    if (pos == std::string::npos) return "";
    pos++;
    
    size_t endPos = json.find("\"", pos);
    if (endPos == std::string::npos) return "";
    
    return json.substr(pos, endPos - pos);
}

static float GetJSONFloatValue(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return 0.0f;
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return 0.0f;
    pos++;
    
    // Pomiń whitespace
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) pos++;
    
    std::string numStr;
    while (pos < json.length() && (isdigit(json[pos]) || json[pos] == '.' || json[pos] == '-')) {
        numStr += json[pos++];
    }
    
    return numStr.empty() ? 0.0f : std::stof(numStr);
}

// Funkcja ładująca teksturę TGA
unsigned int TerrainLoader::LoadTexture(const std::string& filename) {
    std::cout << "=== LoadTexture ===" << std::endl;
    std::cout << "Ładowanie tekstury: " << filename << std::endl;
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "BŁĄD: Nie można otworzyć tekstury: " << filename << std::endl;
        return 0;
    }
    
    std::cout << "Plik otwarty, czytanie nagłówka TGA..." << std::endl;
    
    TGAHeader header;
    file.read((char*)&header, sizeof(TGAHeader));
    
    std::cout << "TGA Header:" << std::endl;
    std::cout << "  imageType: " << (int)header.imageType << std::endl;
    std::cout << "  width: " << header.width << std::endl;
    std::cout << "  height: " << header.height << std::endl;
    std::cout << "  pixelDepth: " << (int)header.pixelDepth << std::endl;
    
    // Wspieramy tylko nieskompresowane RGB/RGBA (typ 2)
    if (header.imageType != 2 || (header.pixelDepth != 24 && header.pixelDepth != 32)) {
        std::cerr << "BŁĄD: Nieobsługiwany format TGA (typ: " << (int)header.imageType 
                  << ", głębia: " << (int)header.pixelDepth << ")" << std::endl;
        std::cerr << "  Obsługiwane: typ=2 (Uncompressed RGB), głębia=24 lub 32" << std::endl;
        return 0;
    }
    
    // Pomiń ID jeśli jest
    if (header.idLength > 0) {
        file.seekg(header.idLength, std::ios::cur);
    }
    
    int width = header.width;
    int height = header.height;
    int bytesPerPixel = header.pixelDepth / 8;
    int imageSize = width * height * bytesPerPixel;
    
    unsigned char* imageData = new unsigned char[imageSize];
    file.read((char*)imageData, imageSize);
    file.close();
    
    // TGA używa BGR/BGRA, więc zamieniamy na RGB/RGBA
    for (int i = 0; i < imageSize; i += bytesPerPixel) {
        unsigned char temp = imageData[i];
        imageData[i] = imageData[i + 2];
        imageData[i + 2] = temp;
    }
    
    // Utwórz teksturę OpenGL
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    GLenum format = (bytesPerPixel == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    delete[] imageData;
    
    std::cout << "Tekstura załadowana pomyślnie: " << width << "x" << height 
              << ", format=" << (bytesPerPixel == 4 ? "RGBA" : "RGB")
              << ", OpenGL ID=" << textureID << std::endl;
    std::cout << "===================" << std::endl;
    return textureID;
}

bool TerrainLoader::LoadTerrain(const std::string& filename, TerrainData& terrain) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku: " << filename << std::endl;
        return false;
    }
    
    std::cout << "Ładowanie terenu z: " << filename << std::endl;
    
    // Czytaj cały plik do stringa
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    // Sprawdź czy to JSON (zaczyna się od '{')
    bool isJSON = false;
    for (char c : content) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') continue;
        if (c == '{') isJSON = true;
        break;
    }
    
    if (isJSON) {
        std::cout << "Wykryto format JSON, parsowanie..." << std::endl;
        
        // Parsuj JSON (bardzo prosty parser)
        std::string heightmapPath = GetJSONStringValue(content, "heightmap");
        float horizontalScale = GetJSONFloatValue(content, "horizontal");
        float verticalScale = GetJSONFloatValue(content, "vertical");
        std::string baseTexturePath = GetJSONStringValue(content, "base");
        
        if (heightmapPath.empty()) {
            std::cerr << "Nie znaleziono ścieżki heightmap w JSON" << std::endl;
            return false;
        }
        
        std::cout << "  heightmap: " << heightmapPath << std::endl;
        std::cout << "  horizontal scale: " << horizontalScale << std::endl;
        std::cout << "  vertical scale: " << verticalScale << std::endl;
        std::cout << "  base texture: " << baseTexturePath << std::endl;
        
        // Zbuduj pełną ścieżkę do PGM (względem katalogu .world)
        std::string worldDir = filename.substr(0, filename.find_last_of("/\\"));
        std::string basePath = worldDir.substr(0, worldDir.find_last_of("/\\"));  // Wyjdź z /maps do /my_diablo_output
        std::string pgmPath = basePath + "/" + heightmapPath;
        
        // Zamień backslashe na forwardslashe
        std::replace(pgmPath.begin(), pgmPath.end(), '\\', '/');
        
        std::cout << "Szukam PGM w: " << pgmPath << std::endl;
        
        // Załaduj PGM
        if (!LoadPGM(pgmPath, terrain.width, terrain.height, terrain.heightMap)) {
            return false;
        }
        
        terrain.cellSize = horizontalScale;
        terrain.verticalScale = verticalScale;
        
        // Załaduj teksturę jeśli podana
        terrain.hasTexture = false;
        terrain.baseTexture = 0;
        
        if (!baseTexturePath.empty()) {
            // Dodaj rozszerzenie .tga jeśli nie ma rozszerzenia
            if (baseTexturePath.find('.') == std::string::npos) {
                baseTexturePath += ".tga";
            } else {
                // Zamień .jpg na .tga (TGA jest prostsze do załadowania)
                size_t extPos = baseTexturePath.find(".jpg");
                if (extPos != std::string::npos) {
                    baseTexturePath.replace(extPos, 4, ".tga");
                }
            }
            
            std::string texturePath = basePath + "/" + baseTexturePath;
            std::replace(texturePath.begin(), texturePath.end(), '\\', '/');
            
            std::cout << "Próba załadowania tekstury: " << texturePath << std::endl;
            
            terrain.baseTexture = LoadTexture(texturePath);
            if (terrain.baseTexture != 0) {
                terrain.hasTexture = true;
                std::cout << "Załadowano teksturę terenu (ID: " << terrain.baseTexture << ")" << std::endl;
            } else {
                std::cout << "UWAGA: Nie udało się załadować tekstury!" << std::endl;
            }
        }
        
    } else {
        std::cout << "Wykryto format tekstowy, parsowanie..." << std::endl;
        
        // Stary format tekstowy
        std::istringstream iss(content);
        char header[256];
        iss.getline(header, sizeof(header));
        
        if (strncmp(header, "WORLD_MAP", 9) != 0) {
            std::cerr << "Nieprawidłowy format pliku (brak nagłówka WORLD_MAP)" << std::endl;
            return false;
        }
        
        // Parsuj parametry
        std::string line;
        while (std::getline(iss, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            std::istringstream lineStream(line);
            std::string key;
            lineStream >> key;
            
            if (key == "width") {
                lineStream >> terrain.width;
            } else if (key == "height") {
                lineStream >> terrain.height;
            } else if (key == "cellSize") {
                lineStream >> terrain.cellSize;
            } else if (key == "verticalScale") {
                lineStream >> terrain.verticalScale;
            } else if (key == "DATA") {
                break;
            }
        }
        
        // Alokuj pamięć na mapę wysokości
        int totalPoints = terrain.width * terrain.height;
        terrain.heightMap = new float[totalPoints];
        
        // Czytaj dane wysokości
        for (int i = 0; i < totalPoints; i++) {
            if (!(iss >> terrain.heightMap[i])) {
                std::cerr << "Błąd odczytu danych wysokości w punkcie " << i << std::endl;
                delete[] terrain.heightMap;
                terrain.heightMap = nullptr;
                return false;
            }
        }
    }
    
    std::cout << "Pomyślnie załadowano teren:" << std::endl;
    std::cout << "  width: " << terrain.width << std::endl;
    std::cout << "  height: " << terrain.height << std::endl;
    std::cout << "  cellSize: " << terrain.cellSize << std::endl;
    std::cout << "  verticalScale: " << terrain.verticalScale << std::endl;
    std::cout << "  total points: " << (terrain.width * terrain.height) << std::endl;
    
    return true;
}

float TerrainLoader::GetHeightAt(const TerrainData& terrain, float worldX, float worldY) {
    if (!terrain.heightMap) return 0.0f;
    
    // Konwertuj współrzędne świata na współrzędne siatki
    float gridX = worldX / terrain.cellSize;
    float gridY = worldY / terrain.cellSize;
    
    // Znajdź komórkę siatki
    int x0 = (int)floorf(gridX);
    int y0 = (int)floorf(gridY);
    
    // Sprawdź granice
    if (x0 < 0 || x0 >= terrain.width - 1 || y0 < 0 || y0 >= terrain.height - 1) {
        return 0.0f; // Poza terenem
    }
    
    // Oblicz wagi interpolacji
    float fx = gridX - x0;
    float fy = gridY - y0;
    
    // Pobierz cztery narożniki komórki
    float h00 = terrain.heightMap[y0 * terrain.width + x0];
    float h10 = terrain.heightMap[y0 * terrain.width + (x0 + 1)];
    float h01 = terrain.heightMap[(y0 + 1) * terrain.width + x0];
    float h11 = terrain.heightMap[(y0 + 1) * terrain.width + (x0 + 1)];
    
    // Interpolacja biliniowa
    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;
    float height = h0 * (1.0f - fy) + h1 * fy;
    
    return height * terrain.verticalScale;
}

Vec3 TerrainLoader::GetNormalAt(const TerrainData& terrain, float worldX, float worldY) {
    // Próbkuj wysokość w czterech punktach wokół pozycji
    const float delta = 1.0f;
    float h_px = GetHeightAt(terrain, worldX + delta, worldY);
    float h_nx = GetHeightAt(terrain, worldX - delta, worldY);
    float h_py = GetHeightAt(terrain, worldX, worldY + delta);
    float h_ny = GetHeightAt(terrain, worldX, worldY - delta);
    
    // Oblicz gradient (różnice wysokości)
    Vec3 normal;
    normal.x = h_nx - h_px;  // Gradient wzdłuż X
    normal.y = h_ny - h_py;  // Gradient wzdłuż Y
    normal.z = 2.0f * delta; // Stała składowa pionowa
    
    normal.normalize();
    return normal;
}
