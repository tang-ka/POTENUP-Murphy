@echo off
REM Launcher: same game windows as original, log consoles moved to bottom half.
REM All logic lives in RunMultiplayerTest_LogPos.ps1 (robust new-window detection).
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0RunMultiplayerTest_LogPos.ps1"
