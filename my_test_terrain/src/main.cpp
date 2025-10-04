#include <windows.h>
#include <GL/gl.h>
#include <iostream>
#include <ctime>

#include "terrain_loader.h"
#include "renderer.h"
#include "player.h"

// Globalne zmienne
HWND g_hWnd = NULL;
HDC g_hDC = NULL;
HGLRC g_hRC = NULL;
bool g_running = true;
int g_windowWidth = 1280;
int g_windowHeight = 720;

// Stan klawiszy
bool g_keys[256] = {false};
bool g_mouseGrabbed = false;
int g_lastMouseX = 0;
int g_lastMouseY = 0;

// Funkcje pomocnicze OpenGL
bool InitOpenGL() {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,  // Color depth
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        24, // Depth buffer
        8,  // Stencil buffer
        0,
        PFD_MAIN_PLANE,
        0, 0, 0, 0
    };
    
    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    if (!pixelFormat) {
        std::cerr << "ChoosePixelFormat failed" << std::endl;
        return false;
    }
    
    if (!SetPixelFormat(g_hDC, pixelFormat, &pfd)) {
        std::cerr << "SetPixelFormat failed" << std::endl;
        return false;
    }
    
    g_hRC = wglCreateContext(g_hDC);
    if (!g_hRC) {
        std::cerr << "wglCreateContext failed" << std::endl;
        return false;
    }
    
    if (!wglMakeCurrent(g_hDC, g_hRC)) {
        std::cerr << "wglMakeCurrent failed" << std::endl;
        return false;
    }
    
    // Ustaw viewport
    glViewport(0, 0, g_windowWidth, g_windowHeight);
    
    std::cout << "OpenGL initialized successfully" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    return true;
}

void CleanupOpenGL() {
    if (g_hRC) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hRC);
        g_hRC = NULL;
    }
    if (g_hDC) {
        ReleaseDC(g_hWnd, g_hDC);
        g_hDC = NULL;
    }
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            return 0;
            
        case WM_CLOSE:
            g_running = false;
            PostQuitMessage(0);
            return 0;
            
        case WM_KEYDOWN:
            if (wParam < 256) {
                g_keys[wParam] = true;
            }
            if (wParam == VK_ESCAPE) {
                g_running = false;
                PostQuitMessage(0);
            }
            return 0;
            
        case WM_KEYUP:
            if (wParam < 256) {
                g_keys[wParam] = false;
            }
            return 0;
            
        case WM_LBUTTONDOWN:
            // Grab mouse
            g_mouseGrabbed = true;
            ShowCursor(FALSE);
            POINT pt;
            GetCursorPos(&pt);
            g_lastMouseX = pt.x;
            g_lastMouseY = pt.y;
            return 0;
            
        case WM_RBUTTONDOWN:
            // Release mouse
            g_mouseGrabbed = false;
            ShowCursor(TRUE);
            return 0;
            
        case WM_SIZE:
            g_windowWidth = LOWORD(lParam);
            g_windowHeight = HIWORD(lParam);
            glViewport(0, 0, g_windowWidth, g_windowHeight);
            return 0;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

// Utwórz okno
bool CreateGameWindow(HINSTANCE hInstance) {
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "TerrainTestWindow";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wc)) {
        std::cerr << "RegisterClassEx failed" << std::endl;
        return false;
    }
    
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    
    RECT windowRect;
    windowRect.left = 0;
    windowRect.right = g_windowWidth;
    windowRect.top = 0;
    windowRect.bottom = g_windowHeight;
    AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);
    
    g_hWnd = CreateWindowEx(
        dwExStyle,
        "TerrainTestWindow",
        "Terrain Collision Test",
        dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        100, 100,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hWnd) {
        std::cerr << "CreateWindowEx failed" << std::endl;
        return false;
    }
    
    g_hDC = GetDC(g_hWnd);
    if (!g_hDC) {
        std::cerr << "GetDC failed" << std::endl;
        return false;
    }
    
    ShowWindow(g_hWnd, SW_SHOW);
    SetForegroundWindow(g_hWnd);
    SetFocus(g_hWnd);
    
    return true;
}

// Główna funkcja
int main(int argc, char* argv[]) {
    // Konsola dla debugowania
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    
    std::cout << "=== Terrain Collision Test ===" << std::endl;
    std::cout << "Argumenty: " << argc << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << "  argv[" << i << "]: " << argv[i] << std::endl;
    }
    std::cout << std::endl;
    
    // Sprawdź czy podano ścieżkę do mapy jako argument
    std::string mapPath;
    if (argc >= 2) {
        mapPath = argv[1];
        std::cout << "Używam mapy z argumentu: " << mapPath << std::endl;
    } else {
        // Domyślna ścieżka
        mapPath = "maps/my_level3.world";
        std::cout << "Brak argumentu, używam domyślnej ścieżki: " << mapPath << std::endl;
        std::cout << "Użycie: test_terrain.exe <ścieżka_do_mapy.world>" << std::endl;
    }
    std::cout << std::endl;
    
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    // Utwórz okno
    if (!CreateGameWindow(hInstance)) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }
    
    // Inicjalizuj OpenGL
    if (!InitOpenGL()) {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        return 1;
    }
    
    // Załaduj teren
    TerrainData terrain;
    if (!TerrainLoader::LoadTerrain(mapPath, terrain)) {
        std::cerr << "Failed to load terrain from: " << mapPath << std::endl;
        
        std::string errorMsg = "Nie można załadować mapy!\n\nŚcieżka: " + mapPath + 
                               "\n\nSprawdź:\n1. Czy plik istnieje\n2. Czy ścieżka jest poprawna\n3. Working directory";
        MessageBox(NULL, errorMsg.c_str(), "Błąd", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Ustaw początkową pozycję gracza w środku mapy
    Player player;
    player.position.x = (terrain.width * terrain.cellSize) / 2.0f;
    player.position.y = (terrain.height * terrain.cellSize) / 2.0f;
    player.position.z = 200.0f;  // Start wysoko, żeby spaść na teren
    
    std::cout << "Pozycja startowa gracza: (" << player.position.x << ", " 
              << player.position.y << ", " << player.position.z << ")" << std::endl;
    
    // Renderer
    Renderer renderer;
    
    // Główna pętla gry
    clock_t lastTime = clock();
    
    std::cout << "\n=== STEROWANIE ===" << std::endl;
    std::cout << "WASD - ruch" << std::endl;
    std::cout << "SPACJA - skok" << std::endl;
    std::cout << "LPM - przytrzymaj i poruszaj myszą aby rozglądać się" << std::endl;
    std::cout << "PPM - zwolnij mysz" << std::endl;
    std::cout << "ESC - wyjście" << std::endl;
    std::cout << "==================\n" << std::endl;
    
    while (g_running) {
        // Obsłuż wiadomości Windows
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Oblicz deltaTime
        clock_t currentTime = clock();
        float deltaTime = (float)(currentTime - lastTime) / CLOCKS_PER_SEC;
        lastTime = currentTime;
        
        // Ogranicz deltaTime (na wypadek pauzy/debuggera)
        if (deltaTime > 0.1f) deltaTime = 0.1f;
        
        // Obsłuż mysz
        if (g_mouseGrabbed) {
            POINT pt;
            GetCursorPos(&pt);
            
            int deltaX = pt.x - g_lastMouseX;
            int deltaY = pt.y - g_lastMouseY;
            
            if (deltaX != 0 || deltaY != 0) {
                float sensitivity = 0.2f;
                player.RotateView(deltaY * sensitivity, deltaX * sensitivity);
                
                // Wyśrodkuj kursor
                SetCursorPos(g_lastMouseX, g_lastMouseY);
            }
        }
        
        // Obsłuż klawisze
        bool forward = g_keys['W'];
        bool back = g_keys['S'];
        bool left = g_keys['A'];
        bool right = g_keys['D'];
        bool jump = g_keys[VK_SPACE];
        
        player.ProcessInput(forward, back, left, right, jump, deltaTime);
        
        // Aktualizuj gracza (fizyka + kolizja)
        player.Update(deltaTime, terrain);
        
        // Renderuj
        renderer.RenderScene(terrain, player);
        
        // Zamień bufory
        SwapBuffers(g_hDC);
        
        // Ograniczenie FPS (~60 FPS)
        Sleep(16);
    }
    
    // Cleanup
    CleanupOpenGL();
    DestroyWindow(g_hWnd);
    FreeConsole();
    
    return 0;
}
