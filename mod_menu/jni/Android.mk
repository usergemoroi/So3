LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := modmenu
LOCAL_SRC_FILES := main.cpp imgui/imgui.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/imgui
LOCAL_LDLIBS := -llog -lGLESv3 -lEGL -landroid -ldl
LOCAL_CPPFLAGS := -std=c++17 -fexceptions -frtti -w -Wno-error
LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
