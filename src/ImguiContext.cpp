#include "ImguiContext.h"
namespace FCT {
    bool ImguiContext::enableChinese(float size,const char* fontPath)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath, size, nullptr,
            io.Fonts->GetGlyphRangesChineseFull());
        if (font == nullptr) {
            return false;
        }
        io.FontDefault = font;
        io.Fonts->Build();
        return true;
    }
}