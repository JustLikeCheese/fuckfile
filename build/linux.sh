#!/bin/bash
set -e

echo "=== Building fuckfile for Linux ==="

# x64
echo "Building x64..."
gcc -o fuckfile-linux-x64 main.c -O2 -static
strip fuckfile-linux-x64
chmod +x fuckfile-linux-x64
echo "✓ fuckfile-linux-x64"

# x86
echo "Building x86..."
gcc -m32 -o fuckfile-linux-x86 main.c -O2 -static
strip fuckfile-linux-x86
chmod +x fuckfile-linux-x86
echo "✓ fuckfile-linux-x86"

# ARM64
echo "Building ARM64..."
aarch64-linux-gnu-gcc -o fuckfile-linux-arm64 main.c -O2 -static
aarch64-linux-gnu-strip fuckfile-linux-arm64
chmod +x fuckfile-linux-arm64
echo "✓ fuckfile-linux-arm64"

# ARMv7
echo "Building ARMv7..."
arm-linux-gnueabihf-gcc -o fuckfile-linux-armv7 main.c -O2 -static
arm-linux-gnueabihf-strip fuckfile-linux-armv7
chmod +x fuckfile-linux-armv7
echo "✓ fuckfile-linux-armv7"

echo "=== All Linux builds completed ==="