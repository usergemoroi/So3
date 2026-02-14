cmake_minimum_required(VERSION 3.20)
project(DiagnosticModule VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ARM64 Android toolchain configuration
if(NOT DEFINED CMAKE_ANDROID_ARCH_ABI)
    set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fvisibility=hidden -fvisibility-inlines-hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -fno-stack-protector")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--strip-all -Wl,--exclude-libs,ALL")

# ImGui source files
set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/backends/imgui_impl_android.cpp
)

# Module sources
set(MODULE_SOURCES
    core/memory.cpp
    core/entity_manager.cpp
    renderer/imgui_overlay.cpp
    renderer/render_backend.cpp
    features/scene_visualizer.cpp
    features/camera_analyzer.cpp
    hooks/render_hooks.cpp
    hooks/input_hooks.cpp
    utils/obfuscation.cpp
    utils/string_encryption.cpp
    main.cpp
)

add_library(${PROJECT_NAME} SHARED ${MODULE_SOURCES} ${IMGUI_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
)

target_link_libraries(${PROJECT_NAME}
    log
    android
    EGL
    GLESv3
    z
)

# Hide symbols
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
