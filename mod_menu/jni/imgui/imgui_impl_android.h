#ifndef IMGUI_IMPL_ANDROID_H
#define IMGUI_IMPL_ANDROID_H

struct AInputEvent;

namespace ImGui_ImplAndroid {
    bool Init(void* window);
    void Shutdown();
    void NewFrame();
    int32_t HandleInputEvent(AInputEvent* input_event);
}

inline bool ImGui_ImplAndroid_Init(void* window) {
    return true;
}

inline void ImGui_ImplAndroid_Shutdown() {}
inline void ImGui_ImplAndroid_NewFrame() {}
inline int32_t ImGui_ImplAndroid_HandleInputEvent(AInputEvent* input_event) {
    return 0;
}

#endif
