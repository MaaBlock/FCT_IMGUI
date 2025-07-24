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
#ifdef FCT_USE_GLFW
        if (dynamic_cast<GLFW_Window*>(wnd))
        {
#ifdef  FCT_USE_VULKAN
            if (dynamic_cast<VK_Context*>(ctx))
            {
                return new GLFW_VK_ImguiContext(
                    dynamic_cast<GLFW_Window*>(wnd),
                    dynamic_cast<VK_Context*>(ctx)
                    );
            }
#endif
        }
#endif

    }


    double ImguiJob::m_lastFrameTime = 0;
   double ImguiJob::m_targetFrameTime = 0;
   bool ImguiJob::m_enableFrameLimiter = true;
}
