@echo off
REM Skrypt uruchamiający grę Diablo Mod z prawidłowymi ustawieniami DLL

echo ====================================================
echo Uruchamianie ioquake3 Diablo Mod
echo ====================================================
echo.
echo Ustawienia:
echo   - fs_game: my_diablo_output
echo   - vm_cgame: 0 (DLL zamiast QVM)
echo   - vm_ui: 0 (DLL zamiast QVM)
echo   - vm_game: 0 (DLL zamiast QVM)
echo   - sv_pure: 0 (mod enabled)
echo.
echo Ladowanie mapy: my_level.world
echo ====================================================
echo.

cd /d "%~dp0build\Debug\my_diablo_output"

start ioquake3_diablo.exe +set fs_basepath "%~dp0build\Debug" +set fs_game my_diablo_output +set sv_pure 0 +set vm_cgame 0 +set vm_ui 0 +set vm_game 0 +set developer 1 +set logfile 2 +world my_level

echo Gra uruchomiona! Sprawdz log w:
echo C:\Users\%USERNAME%\AppData\Roaming\Quake3\my_diablo_output\qconsole.log
echo.
