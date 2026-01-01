#!/bin/bash

ARCH=$1
BINARY=$2

if [ -z "$ARCH" ] || [ -z "$BINARY" ]; then
    echo "Usage: $0 <arch> <binary>"
    exit 1
fi

# Set QEMU runner
case "$ARCH" in
    arm64)
        RUNNER="qemu-aarch64-static"
        ;;
    armv7)
        RUNNER="qemu-arm-static"
        ;;
    x86_64)
        RUNNER="qemu-x86_64-static"
        ;;
    *)
        echo "Unknown architecture: $ARCH"
        exit 1
        ;;
esac

echo "=== fuckfile Test - Android ${ARCH} ==="
echo "Date: $(date)"
echo "Binary: $BINARY"
echo "Emulator: $RUNNER"
echo ""

# Test 1: Help
echo "Test 1: Display help"
$RUNNER ./$BINARY
echo ""

# Test 2: Single file with preserved size and time
echo "Test 2: Fuck single file (preserve size & time)"
echo "Hello World" > test1.txt
ORIG_SIZE=$(stat -c%s test1.txt)
ORIG_TIME=$(stat -c%Y test1.txt)
$RUNNER ./$BINARY -f test1.txt
NEW_SIZE=$(stat -c%s test1.txt)
NEW_TIME=$(stat -c%Y test1.txt)
CONTENT=$(xxd -l 16 test1.txt 2>/dev/null | head -1 || echo "binary content")
echo "Original size: $ORIG_SIZE, New size: $NEW_SIZE"
echo "Time preserved: $([ $ORIG_TIME -eq $NEW_TIME ] && echo YES || echo NO)"
echo "Content (hex): $CONTENT"
echo ""

# Test 3: File with truncate
echo "Test 3: Fuck file with truncate (-s)"
echo "Test content" > test2.txt
$RUNNER ./$BINARY -f -s test2.txt
NEW_SIZE=$(stat -c%s test2.txt)
echo "New size: $NEW_SIZE (should be 0)"
echo ""

# Test 4: Aggressive mode
echo "Test 4: Aggressive mode (-a)"
echo "Aggressive test" > test3.txt
ORIG_TIME=$(stat -c%Y test3.txt)
sleep 1
$RUNNER ./$BINARY -f -a test3.txt
NEW_TIME=$(stat -c%Y test3.txt)
NEW_SIZE=$(stat -c%s test3.txt)
echo "Time changed: $([ $ORIG_TIME -ne $NEW_TIME ] && echo YES || echo NO)"
echo "Size is 0: $([ $NEW_SIZE -eq 0 ] && echo YES || echo NO)"
echo ""

# Test 5: Dry run
echo "Test 5: Dry run mode (-d)"
echo "Dry run test" > test4.txt
ORIG_CONTENT=$(cat test4.txt)
$RUNNER ./$BINARY -f -d test4.txt
NEW_CONTENT=$(cat test4.txt)
echo "Content unchanged: $([ "$ORIG_CONTENT" = "$NEW_CONTENT" ] && echo YES || echo NO)"
echo ""

# Test 6: Recursive directory
echo "Test 6: Recursive directory (-r)"
mkdir -p testdir/subdir
echo "File 1" > testdir/file1.txt
echo "File 2" > testdir/file2.txt
echo "File 3" > testdir/subdir/file3.txt
$RUNNER ./$BINARY -f -r testdir
echo "Files processed:"
find testdir -type f -exec sh -c 'echo "  {}: $(stat -c%s {}) bytes"' \;
echo ""

# Test 7: No preserve time (-t)
echo "Test 7: No preserve time (-t)"
echo "Time test" > test5.txt
ORIG_TIME=$(stat -c%Y test5.txt)
sleep 1
$RUNNER ./$BINARY -f -t test5.txt
NEW_TIME=$(stat -c%Y test5.txt)
echo "Time changed: $([ $ORIG_TIME -ne $NEW_TIME ] && echo YES || echo NO)"
echo ""

echo "=== All tests completed ==="