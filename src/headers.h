//
// Created by Administrator on 2025/4/26.
//

#ifndef FCT_IMGUI_HEADERS_H
#define FCT_IMGUI_HEADERS_H
#include "./ThirdParty.h"
#include "ImguiContext.h"
namespace FCT {
    class ImguiModule
    {
    protected:
        static bool s_initialized;
    public:
        static void Init();
        static void Term();
        ImguiModule();
        ~ImguiModule();
        ImguiContext* createContext(Window* wnd,Context* ctx);
    };
    void InitImgui();
}

#endif //FCT_IMGUI_HEADERS_H
