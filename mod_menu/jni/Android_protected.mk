LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libgamecore
LOCAL_SRC_FILES := main_protected.cpp imgui/imgui.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/imgui
LOCAL_LDLIBS := -llog -lGLESv3 -lEGL -landroid -ldl
LOCAL_CPPFLAGS := -std=c++17 \
                  -fexceptions \
                  -frtti \
                  -w \
                  -Wno-error \
                  -fvisibility=hidden \
                  -fvisibility-inlines-hidden \
                  -ffunction-sections \
                  -fdata-sections \
                  -fno-stack-protector \
                  -fomit-frame-pointer \
                  -O3 \
                  -DNDEBUG \
                  -fno-unwind-tables \
                  -fno-asynchronous-unwind-tables \
                  -Wl,--gc-sections \
                  -Wl,--strip-all \
                  -Wl,-z,norelro \
                  -Wl,--hash-style=gnu

LOCAL_LDFLAGS := -Wl,--gc-sections \
                 -Wl,--strip-all \
                 -Wl,-z,now \
                 -Wl,-z,relro \
                 -Wl,--exclude-libs,ALL

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
