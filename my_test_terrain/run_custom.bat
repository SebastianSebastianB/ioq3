@echo off
REM Uniwersalny launcher - pozwala podać własną ścieżkę do mapy

echo === Terrain Collision Test - Universal Launcher ===
echo.

if "%1"=="" (
    echo UZYCIE: run_custom.bat ^<sciezka_do_mapy.world^>
    echo.
    echo PRZYKLAD:
    echo   run_custom.bat C:\path\to\my_map.world
    echo   run_custom.bat maps\test.world
    echo.
    pause
    exit /b 1
)

REM Sprawdź czy executable istnieje
if not exist "build\bin\Debug\test_terrain.exe" (
    echo BLAD: Nie znaleziono test_terrain.exe
    echo Czy projekt zostal skompilowany?
    echo Uruchom: rebuild.bat
    pause
    exit /b 1
)

REM Sprawdź czy podana mapa istnieje
if not exist "%1" (
    echo BLAD: Nie znaleziono mapy: %1
    echo Sprawdz czy sciezka jest poprawna.
    pause
    exit /b 1
)

echo Mapa: %1
echo.
echo Uruchamianie...
echo.

"build\bin\Debug\test_terrain.exe" "%1"

echo.
echo Program zakonczony.
pause
