#ifndef GLFW_D3D12_IMGUICONTEXT_H
#define GLFW_D3D12_IMGUICONTEXT_H

#include "ImguiContext.h"

namespace FCT
{
    class GLFW_Window;
    class D3D12_Context;

    class GLFW_D3D12_ImguiContext : public ImguiContext
    {
    protected:
        struct SrvDescriptorPair
        {
            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{0};
            D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{0};
        };

        RHI::PassGroup* m_passGroup;
        GLFW_Window* m_wnd;
        D3D12_Context* m_d3dCtx;
        std::queue<UIDeclare> m_uiDeclares[2];
        uint32_t m_currentPush;
        uint32_t m_currentShow;
        std::unordered_map<std::string, Image*> m_textures;
        std::unordered_map<std::string, size_t> m_texturesHash;
        std::unordered_map<std::string, std::vector<SrvDescriptorPair>> m_textureIds;
        std::vector<SrvDescriptorPair> m_freeDescriptors;
        std::mutex m_descriptorMutex;

        SrvDescriptorPair allocSrvDescriptor();
        void freeSrvDescriptor(const SrvDescriptorPair& pair);
        DXGI_FORMAT resolveRtvFormat() const;
        DXGI_FORMAT resolveDsvFormat() const;

        static void srvDescriptorAllocCallback(
            ImGui_ImplDX12_InitInfo* info,
            D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle);
        static void srvDescriptorFreeCallback(
            ImGui_ImplDX12_InitInfo* info,
            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
            D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

    public:
        GLFW_D3D12_ImguiContext(GLFW_Window* wnd, D3D12_Context* ctx);
        ~GLFW_D3D12_ImguiContext() override;

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

#endif // GLFW_D3D12_IMGUICONTEXT_H
