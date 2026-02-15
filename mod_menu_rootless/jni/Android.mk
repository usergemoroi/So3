LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Obfuscated library name: vis -> v
LOCAL_MODULE := v
LOCAL_SRC_FILES := main_noroot.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_LDLIBS := -llog -ldl -landroid -lGLESv2
LOCAL_STATIC_LIBRARIES := c++_static

# Enhanced compiler flags for security
LOCAL_CPPFLAGS := -std=c++17 \
                  -fexceptions \
                  -frtti \
                  -w \
                  -Wno-error \
                  -fvisibility=hidden \
                  -fvisibility-inlines-hidden \
                  -O3 \
                  -DNDEBUG \
                  -DPRODUCTION_BUILD \
                  -ffunction-sections \
                  -fdata-sections \
                  -fno-stack-protector \
                  -fomit-frame-pointer \
                  -fno-unwind-tables \
                  -fno-asynchronous-unwind-tables \
                  -fno-rtti \
                  -fno-exceptions \
                  -s

# Strip all symbols for security
LOCAL_LDFLAGS := -Wl,--gc-sections \
                 -Wl,--strip-all \
                 -Wl,-z,now \
                 -Wl,-z,relro \
                 -Wl,--exclude-libs,ALL

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
