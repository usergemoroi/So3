#ifndef IMGUI_IMPL_OPENGL3_H
#define IMGUI_IMPL_OPENGL3_H

struct ImDrawData;

namespace ImGui_ImplOpenGL3 {
    bool Init(const char* glsl_version = nullptr);
    void Shutdown();
    void NewFrame();
    void RenderDrawData(ImDrawData* draw_data);
}

inline bool ImGui_ImplOpenGL3_Init(const char* glsl_version = nullptr) {
    return true;
}

inline void ImGui_ImplOpenGL3_Shutdown() {}
inline void ImGui_ImplOpenGL3_NewFrame() {}
inline void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data) {}

#endif
