# Unity Game Research Repository

This repository contains Android APK artifacts from a Unity-based mobile game combined with a diagnostic module for research purposes.

## Contents

### Android Application Artifacts
- **APK Components**: Binary manifest, DEX files, resources from decompiled Android application
- **Native Libraries**: ARM64 libraries (Firebase, FMOD, Phonon, Vivox, Unity/Burst)
- **Game Assets**: Audio libraries, advertising SDKs, configuration files

### Diagnostic Module (`diagnostic_module/`)
Professional C++ toolkit for Unity game engine research and analysis on Android ARM64 platforms.

**Features:**
- Memory layout analysis and safe reading
- 3D to 2D projection and rendering
- ESP (Extra Sensory Perception) visualization
- Aim assistance system with smooth targeting
- Anti-detection measures and code obfuscation
- Position-independent code (PIC) for runtime stability

**Components:**
- `include/`: Header files with API definitions
- `src/`: Implementation source files
- `CMakeLists.txt`: Build configuration for Android NDK
- `example_loader.cpp`: Integration example

## ⚠️ Disclaimer

This repository is for **academic research and educational purposes only**:

- Unity engine performance analysis
- Mobile game architecture studies
- Research on false positives in protection systems
- Testing in isolated environments

**Not intended for:**
- Gaining advantages in multiplayer games
- Circumventing anti-cheat systems
- Violating game terms of service

## Technical Details

- **Platform**: Android (ARM64)
- **Game Engine**: Unity with IL2CPP backend
- **Build System**: CMake with Android NDK
- **Language**: C++23
- **Target**: Research and analysis only

## Building Diagnostic Module

```bash
cd diagnostic_module
mkdir build && cd build

# Generate project for ARM64
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-29 \
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --parallel
```

Result: `libdiagnostic_module.so`

## License

This project is for educational and research purposes only.