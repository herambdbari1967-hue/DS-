@echo off
setlocal enabledelayedexpansion

echo ======================================================================
echo  COMPILING AUTO-CORRECT & PREDICTIVE TEXT ENGINE (C DATA STRUCTURES)
echo ======================================================================

:: Check for GCC
where gcc >nul 2>nul
if %errorlevel% equ 0 (
    echo [*] Found GCC compiler. Building modular binary...
    gcc -Wall -Wextra -O2 -Iinclude src/trie.c src/heap.c src/spellcheck.c src/dictionary.c src/main.c -o autocorrect.exe
    if %errorlevel% equ 0 (
        echo [SUCCESS] Built 'autocorrect.exe' successfully using GCC!
        goto done
    )
)

:: Check for Clang
where clang >nul 2>nul
if %errorlevel% equ 0 (
    echo [*] Found Clang compiler. Building modular binary...
    clang -Wall -O2 -Iinclude src/trie.c src/heap.c src/spellcheck.c src/dictionary.c src/main.c -o autocorrect.exe
    if %errorlevel% equ 0 (
        echo [SUCCESS] Built 'autocorrect.exe' successfully using Clang!
        goto done
    )
)

:: Check for MSVC (cl.exe)
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo [*] Found MSVC compiler. Building modular binary...
    cl /W3 /O2 /Iinclude src\trie.c src\heap.c src\spellcheck.c src\dictionary.c src\main.c /Fe:autocorrect.exe
    if %errorlevel% equ 0 (
        echo [SUCCESS] Built 'autocorrect.exe' successfully using MSVC!
        goto done
    )
)

:: Check for TCC (Tiny C Compiler)
where tcc >nul 2>nul
if %errorlevel% equ 0 (
    echo [*] Found TCC compiler. Building modular binary...
    tcc -Iinclude src/trie.c src/heap.c src/spellcheck.c src/dictionary.c src/main.c -o autocorrect.exe
    if %errorlevel% equ 0 (
        echo [SUCCESS] Built 'autocorrect.exe' successfully using TCC!
        goto done
    )
)

echo [!] No standard command-line C compiler found on PATH.
echo [!] You can compile this project in VS Code, Code::Blocks, Dev-C++, or OnlineGDB:
echo     - Modular build: compile all files in 'src/' with include path 'include/'
echo     - Single-file build: compile 'single_file_version/autocorrect_standalone.c'

:done
echo ======================================================================
pause
