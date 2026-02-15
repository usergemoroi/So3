#!/bin/bash
set -e

# Disable memory integrity checks (problematic symbols)
sed -i 's/MemoryIntegrity::Initialize/\/\/ MemoryIntegrity::Initialize/g' ../jni/main_noroot.cpp || true
sed -i 's/MemoryIntegrity::CheckIntegrity/\/\/ MemoryIntegrity::CheckIntegrity/g' ../jni/main_noroot.cpp || true

# Build with NDK
/home/engine/project/mod_menu_rootless/tools_cache/android-ndk-r21e/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk

echo "Build complete!"
ls -lh libs/arm64-v8a/
