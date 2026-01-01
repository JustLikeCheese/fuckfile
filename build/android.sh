#!/bin/bash
set -e

echo "=== Building fuckfile for Android ==="

NDK_BIN="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin"

# ARM64
echo "Building ARM64..."
$NDK_BIN/aarch64-linux-android21-clang -o fuckfile-android-arm64 main.c -O2 -static
$NDK_BIN/llvm-strip fuckfile-android-arm64
chmod +x fuckfile-android-arm64
echo "✓ fuckfile-android-arm64"

# ARMv7
echo "Building ARMv7..."
$NDK_BIN/armv7a-linux-androideabi21-clang -o fuckfile-android-armv7 main.c -O2 -static
$NDK_BIN/llvm-strip fuckfile-android-armv7
chmod +x fuckfile-android-armv7
echo "✓ fuckfile-android-armv7"

# x86_64
echo "Building x86_64..."
$NDK_BIN/x86_64-linux-android21-clang -o fuckfile-android-x86_64 main.c -O2 -static
$NDK_BIN/llvm-strip fuckfile-android-x86_64
chmod +x fuckfile-android-x86_64
echo "✓ fuckfile-android-x86_64"

echo "=== All Android builds completed ==="