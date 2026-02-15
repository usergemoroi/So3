#!/bin/bash

set -e

echo "=========================================="
echo "  FULL BUILD & INJECT"
echo "  classes.dex + libvis.so -> APK"
echo "=========================================="
echo ""

MOD_DIR="/home/engine/project/mod_menu_rootless"

# Переходим в директорию мода
cd "$MOD_DIR"

echo "========== STEP 1: BUILD =========="
echo ""
./build_vis.sh

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed!"
    exit 1
fi

echo ""
echo "========== STEP 2: INJECT =========="
echo ""
./inject_to_apk.sh

if [ $? -ne 0 ]; then
    echo "ERROR: Injection failed!"
    exit 1
fi

echo ""
echo "=========================================="
echo "  COMPLETE!"
echo "=========================================="
echo ""
echo "Summary:"
echo "  ✓ Built classes.dex from Java sources"
echo "  ✓ Built libvis.so from native sources"
echo "  ✓ Injected into extracted APK"
echo ""
echo "Files ready in /home/engine/project:"
echo "  • assets/classes.dex"
echo "  • lib/arm64-v8a/libvis.so"
echo "  • INJECTION_INFO.txt"
echo ""
