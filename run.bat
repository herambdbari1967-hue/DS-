@echo off
if not exist "autocorrect.exe" (
    echo [*] Binary 'autocorrect.exe' not found. Running build.bat first...
    call build.bat
)

if exist "autocorrect.exe" (
    echo [*] Launching Auto-Correct & Predictive Text Engine...
    autocorrect.exe
) else (
    echo [!] Could not run program because 'autocorrect.exe' was not built.
    pause
)
