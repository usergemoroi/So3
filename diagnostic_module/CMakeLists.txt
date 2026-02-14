cmake_minimum_required(VERSION 3.20)
project(diagnostic_module VERSION 1.0.0 LANGUAGES CXX)

# Require C++23 for modern features
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Android NDK configuration
if(ANDROID)
    set(CMAKE_SYSTEM_NAME Android)
    set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
    set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)
endif()

# Compiler flags for ARM64
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    # Optimization flags
    add_compile_options(-O3)
    add_compile_options(-march=armv8-a+crypto+crc)
    
    # Security/hardening (but keep compatible with game environment)
    add_compile_options(-fPIC)
    add_compile_options(-fvisibility=hidden)
    
    # Disable exceptions and RTTI for smaller binary
    add_compile_options(-fno-exceptions)
    add_compile_options(-fno-rtti)
    
    # Linker flags
    add_link_options(-Wl,--exclude-libs,ALL)
    add_link_options(-Wl,--strip-all)
    add_link_options(-Wl,--gc-sections)
    add_link_options(-static-libstdc++)
endif()

# Source files
set(SOURCES
    src/diag_module.cpp
    src/aim_system.cpp
    src/renderer.cpp
    src/hook_impl.cpp
)

# Header files
set(HEADERS
    include/diag_module.hpp
    include/secure_types.hpp
    include/math_ops.hpp
    include/memory_layout.hpp
    include/renderer.hpp
    include/aim_system.hpp
)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external
)

# Link libraries (Android)
if(ANDROID)
    find_library(LOG_LIB log)
    find_library(ANDROID_LIB android)
    find_library(EGL_LIB EGL)
    find_library(GLES_LIB GLESv3)
    
    target_link_libraries(${PROJECT_NAME}
        ${LOG_LIB}
        ${ANDROID_LIB}
        ${EGL_LIB}
        ${GLES_LIB}
    )
endif()

# Compiler definitions
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        DM_EXPORTS
        $<$<CONFIG:Debug>:DM_DEBUG>
        $<$<CONFIG:Release>:DM_RELEASE>
        $<$<CONFIG:Release>:NDEBUG>
)

# ImGui integration (if available)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
    set(IMGUI_SOURCES
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/backends/imgui_impl_android.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
    )
    
    add_library(imgui STATIC ${IMGUI_SOURCES})
    target_include_directories(imgui
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
            ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
    )
    
    target_link_libraries(${PROJECT_NAME} imgui)
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS}
    DESTINATION include/${PROJECT_NAME}
)
