# Skrypt uruchamiający grę Diablo Mod z prawidłowymi ustawieniami DLL

Write-Host "====================================================" -ForegroundColor Cyan
Write-Host "Uruchamianie ioquake3 Diablo Mod" -ForegroundColor Cyan
Write-Host "====================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Ustawienia:" -ForegroundColor Yellow
Write-Host "  - fs_game: my_diablo_output"
Write-Host "  - vm_cgame: 0 (DLL zamiast QVM)" -ForegroundColor Green
Write-Host "  - vm_ui: 0 (DLL zamiast QVM)" -ForegroundColor Green
Write-Host "  - vm_game: 0 (DLL zamiast QVM)" -ForegroundColor Green
Write-Host "  - sv_pure: 0 (mod enabled)"
Write-Host ""
Write-Host "Ładowanie mapy: my_level.world" -ForegroundColor Magenta
Write-Host "====================================================" -ForegroundColor Cyan
Write-Host ""

$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
$basePath = Join-Path $scriptPath "build\Debug"
$gamePath = Join-Path $basePath "my_diablo_output"
$exePath = Join-Path $gamePath "ioquake3_diablo.exe"

if (-not (Test-Path $exePath)) {
    Write-Host "BŁĄD: Nie znaleziono pliku: $exePath" -ForegroundColor Red
    Write-Host "Upewnij się, że gra została skompilowana." -ForegroundColor Red
    pause
    exit 1
}

Set-Location $gamePath

$arguments = @(
    "+set fs_basepath `"$basePath`"",
    "+set fs_game my_diablo_output",
    "+set sv_pure 0",
    "+set vm_cgame 0",
    "+set vm_ui 0",
    "+set vm_game 0",
    "+set developer 1",
    "+set logfile 2",
    "+world my_level"
) -join " "

Start-Process -FilePath $exePath -ArgumentList $arguments

Write-Host ""
Write-Host "Gra uruchomiona!" -ForegroundColor Green
Write-Host ""
Write-Host "Sprawdź log w:" -ForegroundColor Yellow
Write-Host "  C:\Users\$env:USERNAME\AppData\Roaming\Quake3\my_diablo_output\qconsole.log" -ForegroundColor Gray
Write-Host ""
Write-Host "Weryfikacja czy ładują się DLL:" -ForegroundColor Yellow
Write-Host "  Get-Content `$env:APPDATA\Quake3\my_diablo_output\qconsole.log | Select-String 'Loading.*\.dll'" -ForegroundColor Gray
Write-Host ""
