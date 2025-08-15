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

    void ImguiContext::createMainDockSpace(const char* dockspaceName)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin(dockspaceName, nullptr, window_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID(dockspaceName);
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ImGui::End();
    }
}
