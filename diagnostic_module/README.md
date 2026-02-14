# Diagnostic Module for Unity IL2CPP Analysis

## Overview

Internal diagnostic framework for academic research into mobile game engine behavior, rendering pipelines, and entity systems. Designed for ARM64 Android with minimal footprint and stealth considerations.

## Architecture

```
diagnostic_module/
├── core/           # Memory management and entity tracking
├── math/           # Vector/matrix math and view projections
├── features/       # Scene visualization and camera analysis
├── renderer/       # OpenGL/Vulkan render backends and ImGui overlay
├── hooks/          # Render pipeline and input interception
├── utils/          # String encryption and obfuscation
└── main.cpp        # Entry points and lifecycle management
```

## Features

### Scene Visualization
- 2D/3D bounding box rendering
- Skeletal hierarchy visualization
- Health indicator bars
- Trajectory prediction lines
- Distance-based fade culling

### Camera Analysis
- Multiple smoothing algorithms (Linear, Exponential, Bezier, Spring)
- Velocity-based prediction
- FOV-based target filtering
- Latency and jitter metrics
- Bone priority targeting

### Entity Management
- IL2CPP object parsing
- Transform hierarchy tracking
- Bone matrix resolution
- State validation

### Anti-Detection Measures
- Compile-time string encryption (XOR + rotation)
- Control flow obfuscation
- Memory protection awareness
- Minimal syscall usage
- No thread name modifications
- Relative pointer arithmetic only

## Building

### Requirements
- Android NDK r25 or later
- CMake 3.20+
- C++23 support

### Build Commands

```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-26 \
      -DCMAKE_BUILD_TYPE=Release \
      ..
make -j$(nproc)
```

### CMake Options
- `ENABLE_OBFUSCATION`: Enable control flow obfuscation (default: ON)
- `ENABLE_OVERLAY`: Build with ImGui overlay (default: ON)
- `STRIP_SYMBOLS`: Strip debug symbols (default: ON)

## Integration

### Java/Kotlin Loader

```kotlin
class DiagnosticModule {
    companion object {
        init {
            System.loadLibrary("DiagnosticModule")
        }
    }
    
    external fun nativeInit(gameBaseAddr: Long)
    external fun nativeShutdown()
    external fun nativeToggleMenu()
    external fun nativeGetEntityCount(): Int
}
```

### Initialization

The module initializes via JNI from an existing Android context:

1. Load library into process
2. Call `nativeInit()` with module base address
3. Module hooks render pipeline automatically
4. Overlay appears on next frame

## Configuration

### Entity Scan Offsets

Configure per-game offsets in `main.cpp` initialization:

```cpp
core::EntityScanConfig config;
config.entity_list_offset = 0x...;    // Game-specific
config.health_offset = 0x...;         // From IL2CPP dump
config.position_offset = 0x...;       // Transform position
// etc.
```

### Visualization Options

```cpp
features::VisualConfig visual;
visual.show_2d_boxes = true;
visual.show_skeleton = true;
visual.max_render_distance = 500.0f;
```

### Camera Analysis

```cpp
features::CameraAnalyzerConfig cam;
cam.smoothing_type = SmoothingType::Exponential;
cam.smoothing_factor = 0.15f;
cam.use_prediction = true;
```

## Technical Details

### Memory Safety
- All memory reads validated against `/proc/self/maps`
- RAII wrappers for mapped regions
- Volatile access patterns prevent optimization
- Cache-friendly prefetch hints

### Rendering
- OpenGL ES 3.x backend with batch rendering
- Minimal state changes per frame
- VAO/VBO pre-allocation
- Blend-only overlay (no depth/stencil)

### Hook Implementation
- ARM64 inline hooks with trampoline
- Instruction cache coherence maintained
- Original function preservation
- Minimal hook size (4-8 bytes typical)

## Security Considerations

This module is designed for:
- Academic research into game engine architecture
- Performance analysis and profiling
- Educational reverse engineering

**Not intended for:**
- Online game cheating
- Circumventing anti-cheat systems
- Commercial exploitation

Users are responsible for complying with:
- Game Terms of Service
- Local laws regarding reverse engineering
- Academic institution policies

## Performance

- CPU: <1ms per frame typical
- Memory: ~2MB working set
- Draw calls: 1-10 depending on entity count
- Entity limit: 64 (configurable)

## Troubleshooting

### Hook fails to install
- Check SELinux permissive mode
- Verify function alignment
- Ensure target is executable

### No entities detected
- Verify offsets match game version
- Check entity list pointer resolution
- Enable debug logging

### Overlay not visible
- Confirm GL context is current
- Check blend state
- Verify viewport dimensions

## License

Academic use only. See LICENSE for details.

## References

- Unity IL2CPP Internals
- ARM64 Architecture Reference
- OpenGL ES 3.2 Specification
- Android NDK Documentation
