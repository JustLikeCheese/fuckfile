#!/bin/bash
set -e

echo "=== Building fuckfile for Windows (MinGW) ==="

echo "Building x64..."
gcc -o fuckfile-windows-x64.exe main.c -O2 -static
strip fuckfile-windows-x64.exe
echo "✓ fuckfile-windows-x64.exe"

echo "Building x86..."
gcc -o fuckfile-windows-x86.exe main.c -O2 -static
strip fuckfile-windows-x86.exe
echo "✓ fuckfile-windows-x86.exe"

echo "=== All Windows MinGW builds completed ==="