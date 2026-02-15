LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := modmenu_noroot
LOCAL_SRC_FILES := main_noroot.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_LDLIBS := -llog -ldl -landroid
LOCAL_CPPFLAGS := -std=c++17 \
                  -fexceptions \
                  -frtti \
                  -w \
                  -Wno-error \
                  -fvisibility=hidden \
                  -fvisibility-inlines-hidden \
                  -O3 \
                  -DNDEBUG \
                  -ffunction-sections \
                  -fdata-sections

LOCAL_LDFLAGS := -Wl,--gc-sections \
                 -Wl,--strip-all \
                 -Wl,-z,now \
                 -Wl,-z,relro

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
