#!/bin/bash

echo "=========================================="
echo "  Cleanup Script"
echo "=========================================="
echo ""

echo "Removing temporary files..."

rm -rf ../build/
rm -rf ../libs/
rm -rf ../obj/
rm -rf ../output/
rm -rf apk_patch_work/
rm -f *.apk
rm -f *.dex
rm -f *.log

echo "Cleaning NDK build..."
cd ../jni
$ANDROID_NDK_HOME/ndk-build clean 2>/dev/null

cd ..

echo ""
echo "✅ Cleanup complete!"
