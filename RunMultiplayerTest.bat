@echo off
setlocal enabledelayedexpansion
REM Murphy multiplayer LAN session test - 2 independent processes, split screen

set "EXE=C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
set "UPROJECT=C:\tangka\UnrealProjects\POTENUP-Murphy\Murphy.uproject"

REM taskbar margin (raise bottom window by this many px). tune if needed.
set "MARGIN=30"

REM --- detect primary monitor resolution (fallback 1920x1080) ---
set "SW=1920"
set "SH=1080"
set "TMPF=%TEMP%\_murphy_res.txt"
powershell -NoProfile -Command "Add-Type -AssemblyName System.Windows.Forms; $b=[System.Windows.Forms.Screen]::PrimaryScreen.Bounds; \"$($b.Width) $($b.Height)\" | Out-File -Encoding ascii '%TMPF%'"
if exist "%TMPF%" (
    set /p RES=<"%TMPF%"
    for /f "tokens=1,2" %%a in ("!RES!") do (
        set "SW=%%a"
        set "SH=%%b"
    )
    del "%TMPF%" 2>nul
)

REM window size = half monitor; reduce height by margin so taskbar doesn't cover it
set /a HW=%SW%/2
set /a HH=%SH%/2
set /a Y2=%SH%/2 - %MARGIN%


echo Monitor = %SW% x %SH%   Window = %HW% x %HH%
echo.

if not exist "%EXE%" ( echo [ERROR] UnrealEditor.exe not found. & pause & exit /b 1 )
if not exist "%UPROJECT%" ( echo [ERROR] .uproject not found. & pause & exit /b 1 )

echo Launching instance 1 (top-left)...
start "" "%EXE%" "%UPROJECT%" /Game/Maps/Lv_Lobby -game -WINDOWED -ResX=%HW% -ResY=%HH% -WinX=0 -WinY=%MARGIN% -log

timeout /t 3 /nobreak >nul

echo Launching instance 2 (bottom-right)...
start "" "%EXE%" "%UPROJECT%" /Game/Maps/Lv_Lobby -game -WINDOWED -ResX=%HW% -ResY=%HH% -WinX=%HW% -WinY=%MARGIN% -log

echo.
echo Done. If no windows appeared, read messages above.
pause
