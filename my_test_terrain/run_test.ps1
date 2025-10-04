# PowerShell launcher dla Terrain Collision Test
Write-Host "=== Terrain Collision Test - PowerShell Launcher ===" -ForegroundColor Cyan
Write-Host ""

# Pobierz katalog skryptu
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Ścieżka do executable
$ExePath = Join-Path $ScriptDir "build\bin\Debug\test_terrain.exe"

# Sprawdź czy executable istnieje
if (-not (Test-Path $ExePath)) {
    Write-Host "BLAD: Nie znaleziono test_terrain.exe" -ForegroundColor Red
    Write-Host "Sciezka: $ExePath" -ForegroundColor Yellow
    Write-Host "Czy projekt zostal skompilowany?" -ForegroundColor Yellow
    Write-Host "Uruchom: .\rebuild.bat" -ForegroundColor Yellow
    Read-Host "Nacisnij Enter aby zakonczyc"
    exit 1
}

# Ścieżka do mapy
$MapPath = "C:\Users\AI\Desktop\myDebian\git\Quake-III-Arena\ioq3\build\Debug\my_diablo_output\maps\my_level3.world"

# Sprawdź czy mapa istnieje
if (-not (Test-Path $MapPath)) {
    Write-Host "BLAD: Nie znaleziono mapy" -ForegroundColor Red
    Write-Host "Sciezka: $MapPath" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Sprawdz:" -ForegroundColor Yellow
    Write-Host "1. Czy plik mapy istnieje" -ForegroundColor Yellow
    Write-Host "2. Czy sciezka jest poprawna" -ForegroundColor Yellow
    Write-Host "3. Czy mapa zostala wygenerowana (uruchom gre Quake III z modem Diablo)" -ForegroundColor Yellow
    Read-Host "Nacisnij Enter aby zakonczyc"
    exit 1
}

Write-Host "Executable: $ExePath" -ForegroundColor Green
Write-Host "Mapa: $MapPath" -ForegroundColor Green
Write-Host ""
Write-Host "Uruchamianie silnika testowego..." -ForegroundColor Cyan
Write-Host ""

# Uruchom program
& $ExePath $MapPath

$ExitCode = $LASTEXITCODE
Write-Host ""
Write-Host "Program zakonczony (kod: $ExitCode)" -ForegroundColor $(if ($ExitCode -eq 0) { "Green" } else { "Red" })
Read-Host "Nacisnij Enter aby zakonczyc"
