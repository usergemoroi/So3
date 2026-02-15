#include "imgui.h"
#include <cstdio>
#include <cstdarg>
#include <vector>
#include <string>

static ImGuiIO g_IO;
static ImDrawList g_BackgroundDrawList;
static ImDrawData g_DrawData;
static std::vector<bool> g_MouseDown;
static ImVec2 g_MousePos;
static ImVec2 g_CurrentWindowPos;
static ImVec2 g_CurrentWindowSize;
static bool g_CurrentWindowHovered = false;

void ImGui::CreateContext() {
    g_IO.Framerate = 60.0f;
    g_IO.IniFilename = nullptr;
    g_MouseDown.resize(5, false);
}

ImGuiIO& ImGui::GetIO() {
    return g_IO;
}

void ImGui::StyleColorsDark() {
}

void ImGui::NewFrame() {
    g_IO.Framerate = 60.0f;
}

void ImGui::Render() {
}

ImDrawData* ImGui::GetDrawData() {
    return &g_DrawData;
}

ImDrawList* ImGui::GetBackgroundDrawList() {
    return &g_BackgroundDrawList;
}

void ImGui::SetNextWindowPos(const ImVec2& pos, ImGuiCond cond) {
    g_CurrentWindowPos = pos;
}

void ImGui::SetNextWindowSize(const ImVec2& size, ImGuiCond cond) {
    g_CurrentWindowSize = size;
}

bool ImGui::Begin(const char* name, bool* p_open, ImGuiWindowFlags flags) {
    return true;
}

void ImGui::End() {
}

bool ImGui::Checkbox(const char* label, bool* v) {
    return false;
}

bool ImGui::SliderFloat(const char* label, float* v, float v_min, float v_max) {
    return false;
}

bool ImGui::Button(const char* label, const ImVec2& size) {
    return false;
}

void ImGui::Separator() {
}

void ImGui::Text(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
}

bool ImGui::CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags) {
    return true;
}

ImVec2 ImGui::CalcTextSize(const char* text, const char* text_end) {
    if (!text) return ImVec2(0, 0);
    
    int len = 0;
    if (text_end) {
        len = text_end - text;
    } else {
        len = strlen(text);
    }
    
    return ImVec2(len * 8.0f, 16.0f);
}

ImVec2 ImGui::GetMousePos() {
    return g_MousePos;
}

bool ImGui::IsMouseDown(int button) {
    if (button < 0 || button >= (int)g_MouseDown.size()) {
        return false;
    }
    return g_MouseDown[button];
}

bool ImGui::IsWindowHovered() {
    return g_CurrentWindowHovered;
}

ImVec2 ImGui::GetWindowPos() {
    return g_CurrentWindowPos;
}

ImVec2 ImGui::GetWindowSize() {
    return g_CurrentWindowSize;
}

void ImGui::SetWindowPos(const ImVec2& pos) {
    g_CurrentWindowPos = pos;
}
