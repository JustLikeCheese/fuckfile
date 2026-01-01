@echo off
echo === Building fuckfile for Windows ARM64 ===

echo Building ARM64...
cl /O2 main.c /Fe:fuckfile-windows-arm64.exe
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
del main.obj
echo + fuckfile-windows-arm64.exe

echo === Windows ARM64 build completed ===