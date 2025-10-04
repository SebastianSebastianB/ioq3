@echo off
REM Skrypt uruchamiający Terrain Collision Test

echo === Terrain Collision Test - Launcher ===
echo.

REM Pobierz katalog, w którym znajduje się ten skrypt
set SCRIPT_DIR=%~dp0

REM Ścieżka do executable (bezwzględna)
set EXE_PATH=%SCRIPT_DIR%build\bin\Debug\test_terrain.exe

REM Sprawdź czy executable istnieje
if not exist "%EXE_PATH%" (
    echo BLAD: Nie znaleziono test_terrain.exe
    echo Sciezka: %EXE_PATH%
    echo Czy projekt zostal skompilowany?
    echo Uruchom: rebuild.bat
    pause
    exit /b 1
)

REM Ścieżka do mapy (bezwzględna)
set MAP_PATH=C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output\maps\my_level3.world

REM Sprawdź czy mapa istnieje
if not exist "%MAP_PATH%" (
    echo BLAD: Nie znaleziono mapy w: %MAP_PATH%
    echo.
    echo Sprawdz:
    echo 1. Czy plik mapy istnieje
    echo 2. Czy sciezka jest poprawna
    echo 3. Czy mapa zostala wygenerowana (uruchom gre Quake III z modem Diablo)
    echo.
    pause
    exit /b 1
)

echo Executable: %EXE_PATH%
echo Mapa: %MAP_PATH%
echo.

REM Uruchom program z mapą jako parametrem
echo Uruchamianie silnika testowego...
echo.

"%EXE_PATH%" "%MAP_PATH%"

echo.
echo Program zakonczony (kod: %ERRORLEVEL%)
pause
