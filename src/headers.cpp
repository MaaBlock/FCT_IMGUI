//
// Created by Administrator on 2025/4/26.
//
#include "./FCTAPI.h"

namespace FCT
{
    bool ImguiModule::s_initialized = false;
    void ImguiModule::Init()
    {
        ImGui::CreateContext();
        s_initialized = true;
    }

    void ImguiModule::Term()
    {
        ImGui::DestroyContext();
    }

    ImguiModule::ImguiModule()
    {
        if (!s_initialized)
            Init();
    }
    ImguiModule::~ImguiModule()
    {

    }

    ImguiContext* ImguiModule::createContext(Window* wnd, Context* ctx)
    {
#ifdef FCT_WIN32
        if (auto* win32Wnd = dynamic_cast<Win32_Window*>(wnd))
        {
#ifdef FCT_USE_D3D12
            if (auto* d3dCtx = dynamic_cast<D3D12_Context*>(ctx))
            {
                return new Win32_D3D12_ImguiContext(win32Wnd, d3dCtx);
            }
#endif
#ifdef FCT_USE_VULKAN
            if (auto* vkCtx = dynamic_cast<VK_Context*>(ctx))
            {
                return new Win32_VK_ImguiContext(win32Wnd, vkCtx);
            }
#endif
        }
#endif
#ifdef FCT_USE_GLFW
        if (auto* glfwWnd = dynamic_cast<GLFW_Window*>(wnd))
        {
#ifdef FCT_USE_VULKAN
            if (auto* vkCtx = dynamic_cast<VK_Context*>(ctx))
            {
                return new GLFW_VK_ImguiContext(glfwWnd, vkCtx);
            }
#endif
#ifdef FCT_USE_D3D12
            if (auto* d3dCtx = dynamic_cast<D3D12_Context*>(ctx))
            {
                return new GLFW_D3D12_ImguiContext(glfwWnd, d3dCtx);
            }
#endif
        }
#endif
        return nullptr;
    }

/*
    double ImguiJob::m_lastFrameTime = 0;
   double ImguiJob::m_targetFrameTime = 0;
   bool ImguiJob::m_enableFrameLimiter = true;
   */
}
