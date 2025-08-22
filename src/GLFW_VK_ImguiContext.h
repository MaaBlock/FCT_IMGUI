#ifndef VK_GLFW_IMGUICONTEXT_H
#define VK_GLFW_IMGUICONTEXT_H
#include "ImguiContext.h"

struct ImGui_ImplGlfw_Data;

namespace FCT
{
    class GLFW_Window;
    class GLFW_VK_ImguiContext;
    using ImguiTask = std::function<void()>;
    class GLFW_VK_ImguiContext : public ImguiContext
    {
    protected:
        RHI::PassGroup* m_passGroup;
        GLFW_Window* m_wnd;
        VK_Context* m_vkCtx;
        //std::string m_passName;
        std::queue<UIDeclare> m_uiDeclares[2];
        uint32_t m_currentPush;
        uint32_t m_currentShow;
        std::unordered_map<std::string,Image*> m_textures;
        std::unordered_map<std::string,size_t> m_texturesHash;
        std::unordered_map<std::string,std::vector<VkDescriptorSet>> m_textureIds;
        RHI::VK_Sampler* m_sampler;
   public:
        GLFW_VK_ImguiContext(GLFW_Window* wnd,VK_Context* ctx);
        ~GLFW_VK_ImguiContext() override;
        void addTexture(std::string name,MutilBufferImage* image);
        void addTexture(std::string name,SingleBufferImage* image);
        void addTexture(std::string name,Image* image) override;
        void removeTexture(std::string name);
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
    protected:
        void newFrame_updataSize();
        void newFrame_UpdateMouseData();
        void newFrame_updataTime();
        void newFrame_UpdateMonitors();
        void newFrame_updateMouseCursor();
        void newFrame_UpdateGamepads();
        void newFrame_updateInput();
    };

}
#endif //VK_GLFW_IMGUICONTEXT_H
