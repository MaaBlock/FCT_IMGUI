#ifndef WIN32_VK_IMGUICONTEXT_H
#define WIN32_VK_IMGUICONTEXT_H

#include "ImguiContext.h"

namespace FCT
{
    class Win32_Window;
    class VK_Context;

    class Win32_VK_ImguiContext : public ImguiContext
    {
    protected:
        RHI::PassGroup* m_passGroup;
        Win32_Window* m_wnd;
        VK_Context* m_vkCtx;
        std::queue<UIDeclare> m_uiDeclares[2];
        uint32_t m_currentPush;
        uint32_t m_currentShow;
        std::unordered_map<std::string, Image*> m_textures;
        std::unordered_map<std::string, size_t> m_texturesHash;
        std::unordered_map<std::string, std::vector<VkDescriptorSet>> m_textureIds;
        RHI::VK_Sampler* m_sampler;
        uint32_t m_wndProcCallbackId;

    public:
        Win32_VK_ImguiContext(Win32_Window* wnd, VK_Context* ctx);
        ~Win32_VK_ImguiContext() override;

        void addTexture(std::string name, MutilBufferImage* image);
        void addTexture(std::string name, SingleBufferImage* image);
        void addTexture(std::string name, Image* image) override;
        void removeTexture(std::string name) override;
        ImTextureID getTexture(std::string name) override;
        void updateTexture(std::string name) override;
        void push(UIDeclare uiFunc) override;
        void swapBuffer() override;
        void submit(RHI::CommandBuffer* cmdBuf) override;
        void newFrame();
        void submitTick(RHI::CommandBuffer* cmdBuffer);
        void render();
        void drawData(RHI::CommandBuffer* cmdBuffer);
        void updatePassResource() override;
        void createPlatform() override;
        void destroyPlatform() override;
    };
}

#endif // WIN32_VK_IMGUICONTEXT_H
