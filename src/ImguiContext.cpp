#include "ImguiContext.h"
namespace FCT {
    ImguiContext::ImguiContext()
    {
        m_pass = nullptr;
    }

    void ImguiContext::pass(RHI::Pass* pass)
    {
        m_pass = pass;
    }

    void ImguiContext::create(ImguiContextCreateFlags flags)
    {
        ImGuiIO& io = ImGui::GetIO();

        if (flags & ImguiContextCreateFlag::Docking)
        {
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

            io.ConfigDockingWithShift = false;
            io.ConfigDockingAlwaysTabBar = true;
            io.ConfigDockingTransparentPayload = true;
        }

        if (flags & ImguiContextCreateFlag::NavKeyboard)
        {
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        }
        createPlatform();
    }

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