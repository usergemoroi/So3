#ifndef IMGUI_H
#define IMGUI_H

#include <cstdint>
#include <cstring>

#define IMGUI_VERSION "1.89"
#define IMGUI_CHECKVERSION()

typedef uint32_t ImU32;
typedef uint32_t ImGuiID;
typedef int ImGuiCol;
typedef int ImGuiCond;
typedef int ImGuiWindowFlags;
typedef int ImGuiTreeNodeFlags;
typedef int ImDrawFlags;

enum ImGuiCond_ {
    ImGuiCond_None = 0,
    ImGuiCond_Always = 1 << 0,
    ImGuiCond_Once = 1 << 1,
    ImGuiCond_FirstUseEver = 1 << 2,
    ImGuiCond_Appearing = 1 << 3
};

enum ImGuiWindowFlags_ {
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_NoTitleBar = 1 << 0,
    ImGuiWindowFlags_NoResize = 1 << 1,
    ImGuiWindowFlags_NoMove = 1 << 2,
    ImGuiWindowFlags_NoScrollbar = 1 << 3,
    ImGuiWindowFlags_NoCollapse = 1 << 5,
    ImGuiWindowFlags_AlwaysAutoResize = 1 << 6
};

enum ImGuiTreeNodeFlags_ {
    ImGuiTreeNodeFlags_None = 0,
    ImGuiTreeNodeFlags_DefaultOpen = 1 << 5
};

enum ImDrawFlags_ {
    ImDrawFlags_None = 0
};

struct ImVec2 {
    float x, y;
    ImVec2() : x(0.0f), y(0.0f) {}
    ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

struct ImVec4 {
    float x, y, z, w;
    ImVec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

#define IM_COL32(R,G,B,A) (((ImU32)(A)<<24) | ((ImU32)(B)<<16) | ((ImU32)(G)<<8) | ((ImU32)(R)<<0))
#define IM_COL32_WHITE IM_COL32(255,255,255,255)
#define IM_COL32_BLACK IM_COL32(0,0,0,255)

struct ImGuiIO {
    float Framerate;
    const char* IniFilename;
};

struct ImDrawList {
    void AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f) {}
    void AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, ImDrawFlags flags = 0, float thickness = 1.0f) {}
    void AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, ImDrawFlags flags = 0) {}
    void AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = nullptr) {}
    void AddCircle(const ImVec2& center, float radius, ImU32 col, int num_segments = 0, float thickness = 1.0f) {}
    void AddCircleFilled(const ImVec2& center, float radius, ImU32 col, int num_segments = 0) {}
};

struct ImDrawData {
};

namespace ImGui {
    void CreateContext();
    ImGuiIO& GetIO();
    void StyleColorsDark();
    void NewFrame();
    void Render();
    ImDrawData* GetDrawData();
    ImDrawList* GetBackgroundDrawList();
    
    void SetNextWindowPos(const ImVec2& pos, ImGuiCond cond = 0);
    void SetNextWindowSize(const ImVec2& size, ImGuiCond cond = 0);
    bool Begin(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void End();
    
    bool Checkbox(const char* label, bool* v);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max);
    bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
    void Separator();
    void Text(const char* fmt, ...);
    bool CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags = 0);
    
    ImVec2 CalcTextSize(const char* text, const char* text_end = nullptr);
    ImVec2 GetMousePos();
    bool IsMouseDown(int button);
    bool IsWindowHovered();
    ImVec2 GetWindowPos();
    ImVec2 GetWindowSize();
    void SetWindowPos(const ImVec2& pos);
}

#endif
