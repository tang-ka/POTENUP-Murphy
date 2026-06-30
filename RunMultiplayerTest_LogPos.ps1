$ErrorActionPreference = "Stop"

$EXE      = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
$UPROJECT = "C:\PU_Project\_TeamProjects\ALLKILL\POTENUP-Murphy\Murphy.uproject"
$MAP      = "/Game/Maps/Lv_Lobby"
$MARGIN   = 30

# Console host classes (cmd, Windows Terminal, legacy console).
$consoleClasses = @("ConsoleWindowClass", "CASCADIA_HOSTING_WINDOW_CLASS", "PseudoConsoleWindow")

Add-Type @"
using System;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;
public static class Win
{
    [DllImport("user32.dll")] public static extern bool MoveWindow(IntPtr hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);
    [DllImport("user32.dll")] public static extern int GetClassName(IntPtr hWnd, StringBuilder lpClassName, int nMaxCount);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr hWnd);
    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

    public static List<IntPtr> List(string[] classes)
    {
        List<IntPtr> result = new List<IntPtr>();
        EnumWindows(delegate(IntPtr h, IntPtr l)
        {
            if (!IsWindowVisible(h)) { return true; }
            StringBuilder c = new StringBuilder(256);
            GetClassName(h, c, 256);
            string cls = c.ToString();
            foreach (string want in classes)
            {
                if (cls == want) { result.Add(h); break; }
            }
            return true;
        }, IntPtr.Zero);
        return result;
    }
}
"@

function Get-ConsoleSet
{
    $set = New-Object 'System.Collections.Generic.HashSet[System.IntPtr]'
    foreach ($h in [Win]::List($consoleClasses))
    {
        [void]$set.Add($h)
    }
    return $set
}

function Wait-NewConsole
{
    param(
        [System.Collections.Generic.HashSet[System.IntPtr]]$baseline,
        [int]$TimeoutSec = 20
    )
    $deadline = (Get-Date).AddSeconds($TimeoutSec)
    do
    {
        foreach ($h in [Win]::List($consoleClasses))
        {
            if (-not $baseline.Contains($h))
            {
                return $h
            }
        }
        Start-Sleep -Milliseconds 300
    } while ((Get-Date) -lt $deadline)
    return [System.IntPtr]::Zero
}

function Launch-Instance
{
    param(
        [int]$GameX, [int]$GameY, [int]$GameW, [int]$GameH,
        [int]$LogX,  [int]$LogY,  [int]$LogW,  [int]$LogH,
        [string]$Label
    )

    Write-Host "Launching $Label ..."
    $baseline = Get-ConsoleSet

    # ----- GAME WINDOW: position = -WinX/-WinY, size = -ResX/-ResY -----
    $argList = @(
        "`"$UPROJECT`"",
        $MAP,
        "-game",
        "-WINDOWED",
        "-ResX=$GameW",
        "-ResY=$GameH",
        "-WinX=$GameX",
        "-WinY=$GameY",
        "-log"
    )
    Start-Process -FilePath $EXE -ArgumentList $argList | Out-Null

    # ----- LOG WINDOW: find the newly opened console and place it -----
    $log = Wait-NewConsole -baseline $baseline -TimeoutSec 20
    if ($log -ne [System.IntPtr]::Zero)
    {
        [Win]::MoveWindow($log, $LogX, $LogY, $LogW, $LogH, $true) | Out-Null
        Write-Host "  -> log window moved."
    }
    else
    {
        Write-Host "  -> log window not detected within timeout."
    }
}

# =====================================================================
# detect primary monitor resolution (fallback 1920x1080)
# =====================================================================
Add-Type -AssemblyName System.Windows.Forms
$b  = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$SW = $b.Width
$SH = $b.Height
if ($SW -le 0) { $SW = 1920 }
if ($SH -le 0) { $SH = 1080 }

# =====================================================================
# CONFIG  ===  adjust these numbers  ===
# =====================================================================

# ----- GAME WINDOW size (both instances share the same size) -----
$GAME_W = [int]($SW / 2)
$GAME_H = [int]($SH / 2)

# ----- GAME WINDOW position -----
$GAME1_X = 0          ; $GAME1_Y = $MARGIN         # instance 1: top-left
$GAME2_X = $GAME_W    ; $GAME2_Y = $MARGIN         # instance 2: top-right

# ----- LOG WINDOW offsets -----
$LOG_TOP_OFFSET = 40    # push logs down so their top control bar is not hidden by the game window above
$BOTTOM_MARGIN  = 40    # keep logs clear of the taskbar at the very bottom

# ----- LOG WINDOW size -----
$LOG1_W = [int]($SW / 2)        # left log width (exact half)
$LOG2_W = $SW - $LOG1_W         # right log fills the remaining width (no left/right gap)
$LOG_H  = [int]($SH / 2) - $LOG_TOP_OFFSET - $BOTTOM_MARGIN

# ----- LOG WINDOW position -----
$LOG_Y  = [int]($SH / 2) + $LOG_TOP_OFFSET         # top of bottom half + offset
$LOG1_X = 0                                        # instance 1 log: bottom-left
$LOG2_X = $LOG1_W                                  # instance 2 log: bottom-right

Write-Host "Monitor = $SW x $SH   GameWindow = $GAME_W x $GAME_H"
Write-Host ""

if (-not (Test-Path $EXE))      { Write-Host "[ERROR] UnrealEditor.exe not found."; Read-Host "Press Enter"; exit 1 }
if (-not (Test-Path $UPROJECT)) { Write-Host "[ERROR] .uproject not found."; Read-Host "Press Enter"; exit 1 }

# instance 1: game top-left, log bottom-left
Launch-Instance -GameX $GAME1_X -GameY $GAME1_Y -GameW $GAME_W -GameH $GAME_H `
                -LogX $LOG1_X -LogY $LOG_Y -LogW $LOG1_W -LogH $LOG_H `
                -Label "instance 1 (top-left)"

Start-Sleep -Seconds 3

# instance 2: game top-right, log bottom-right
Launch-Instance -GameX $GAME2_X -GameY $GAME2_Y -GameW $GAME_W -GameH $GAME_H `
                -LogX $LOG2_X -LogY $LOG_Y -LogW $LOG2_W -LogH $LOG_H `
                -Label "instance 2 (top-right)"

Write-Host ""
Write-Host "Done."
Read-Host "Press Enter to close"
