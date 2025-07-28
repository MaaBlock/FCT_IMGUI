//
// Created by Administrator on 2025/4/26.
//
#include "./ThirdParty.h"
#ifndef IMGUICONTEXT_H
#define IMGUICONTEXT_H
namespace FCT {
    using UIDeclare = std::function<void(void)>;
    class ImguiContext {
      public:
        /**
         * @brief push the declaration for imgui to the back buffer
         * @param uiFunc declaration for UI rendering
         */
        virtual void push(UIDeclare uiFunc) = 0;
        /**
         * @brief swap front buffer and back buffer
         */
        virtual void swapBuffer() = 0;
        /**
         * @brief submit front buffer to GPU
         * @param cmdBuf the command buffer to submit
         */
        virtual void submit(RHI::CommandBuffer* cmdBuf) = 0;
        /**
         * @brief build imgui context
         * @param pass The pass to create imgui context
         */
        virtual void create(RHI::Pass* pass) = 0;
        bool enableChinese(float size = 18.0f,const char* fontPath = "C:\\Windows\\Fonts\\simsun.ttc");

    private:

    };
}
#endif //IMGUICONTEXT_H
