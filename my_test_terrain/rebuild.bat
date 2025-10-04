@echo off
REM Szybka rekompilacja projektu testowego

echo === Rebuilding Terrain Test Engine ===
echo.

cd build
cmake --build . --config Debug

if %ERRORLEVEL% EQU 0 (
    echo.
    echo === KOMPILACJA UDANA ===
    echo Mozesz uruchomic: run_test.bat
) else (
    echo.
    echo === BLAD KOMPILACJI ===
    echo Sprawdz komunikaty powyzej
)

echo.
pause
