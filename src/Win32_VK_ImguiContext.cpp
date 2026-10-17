#include "./Win32_VK_ImguiContext.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace FCT
{
    Win32_VK_ImguiContext::Win32_VK_ImguiContext(Win32_Window* wnd, VK_Context* ctx)
        : ImguiContext(ctx)
        , m_passGroup(nullptr)
        , m_wnd(wnd)
        , m_vkCtx(ctx)
        , m_currentPush(0)
        , m_currentShow(1)
        , m_sampler(nullptr)
        , m_wndProcCallbackId(0)
    {
        m_ctx = ctx;
    }

    Win32_VK_ImguiContext::~Win32_VK_ImguiContext()
    {
        destroyPlatform();
        m_textures.clear();
    }

    void Win32_VK_ImguiContext::addTexture(std::string name, MutilBufferImage* image)
    {
        m_textures[name] = image;
        if (!m_platformInitialized)
        {
            return;
        }
        std::vector<RHI::TextureView*> rhiTextureViews = image->tvs();
        for (uint32_t i = 0; i < image->imageCount() && i < rhiTextureViews.size(); ++i)
        {
            auto* tv = static_cast<RHI::VK_TextureView*>(rhiTextureViews[i]);
            vk::ImageView imageViewVk = tv->view();
            vk::Sampler samplerVk = m_sampler->getSampler();
            VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(
                samplerVk, imageViewVk, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            m_textureIds[name].push_back(descriptorSet);
        }
    }

    void Win32_VK_ImguiContext::addTexture(std::string name, SingleBufferImage* image)
    {
        m_textures[name] = image;
        if (!m_platformInitialized)
        {
            return;
        }
        auto* tv = static_cast<RHI::VK_TextureView*>(image->currentTextureView());
        vk::ImageView imageViewVk = tv->view();
        vk::Sampler samplerVk = m_sampler->getSampler();
        VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(
            samplerVk, imageViewVk, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_textureIds[name].push_back(descriptorSet);
    }

    void Win32_VK_ImguiContext::addTexture(std::string name, Image* image)
    {
        if (dynamic_cast<MutilBufferImage*>(image))
        {
            addTexture(name, static_cast<MutilBufferImage*>(image));
        }
        else
        {
            addTexture(name, static_cast<SingleBufferImage*>(image));
        }
    }

    void Win32_VK_ImguiContext::removeTexture(std::string name)
    {
        auto it = m_textureIds.find(name);
        if (it != m_textureIds.end())
        {
            for (auto ds : it->second)
            {
                ImGui_ImplVulkan_RemoveTexture(ds);
            }
            m_textureIds.erase(it);
        }
        m_textures.erase(name);
    }

    ImTextureID Win32_VK_ImguiContext::getTexture(std::string name)
    {
        auto it = m_textureIds.find(name);
        if (it != m_textureIds.end() && !it->second.empty())
        {
            return reinterpret_cast<ImTextureID>(it->second[0]);
        }
        return reinterpret_cast<ImTextureID>(nullptr);
    }

    void Win32_VK_ImguiContext::updateTexture(std::string name)
    {
        auto it = m_textures.find(name);
        if (it == m_textures.end())
        {
            return;
        }
        Image* image = it->second;
        removeTexture(name);
        addTexture(name, image);
    }

    void Win32_VK_ImguiContext::push(UIDeclare uiFunc)
    {
        m_uiDeclares[m_currentPush].push(uiFunc);
    }

    void Win32_VK_ImguiContext::swapBuffer()
    {
        std::swap(m_currentPush, m_currentShow);
    }

    void Win32_VK_ImguiContext::submit(RHI::CommandBuffer* cmdBuf)
    {
        ZoneScoped();
        newFrame();
        while (!m_uiDeclares[m_currentShow].empty())
        {
            ZoneScopedN("ImGui::UIDeclare");
            auto uiFunc = m_uiDeclares[m_currentShow].front();
            uiFunc();
            m_uiDeclares[m_currentShow].pop();
        }
        render();
        drawData(cmdBuf);
    }

    void Win32_VK_ImguiContext::newFrame()
    {
        ZoneScoped();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void Win32_VK_ImguiContext::updatePassResource()
    {
        ZoneScoped();
        if (!m_platformInitialized)
        {
            return;
        }
        std::vector<std::string> needUpdate;
        for (const auto& it : m_textures)
        {
            auto hash = it.second->textureViewHash();
            if (m_texturesHash[it.first] != hash)
            {
                m_texturesHash[it.first] = hash;
                needUpdate.push_back(it.first);
            }
        }
        for (const auto& name : needUpdate)
        {
            updateTexture(name);
        }
    }

    void Win32_VK_ImguiContext::createPlatform()
    {
        ZoneScoped();
        m_passGroup = m_pass ? m_pass->group() : nullptr;

        ImGui_ImplWin32_Init(m_wnd->getHwnd());
        m_wndProcCallbackId = m_wnd->addWndProcCallback([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
        {
            return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
        });

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = m_vkCtx->getVkInstance();
        initInfo.PhysicalDevice = m_vkCtx->getPhysicalDevice();
        initInfo.Device = m_vkCtx->getDevice();
        initInfo.QueueFamily = m_vkCtx->getGraphicsQueueFamily();
        initInfo.Queue = m_vkCtx->getGraphicsQueue();
        initInfo.PipelineCache = nullptr;
        initInfo.DescriptorPool = static_cast<RHI::VK_DescriptorPool*>(m_vkCtx->getDescriptorPool())->getPool();
        initInfo.Allocator = nullptr;
        initInfo.MinImageCount = 2;
        initInfo.ImageCount = m_wnd->getSwapchainImageCount() > 0 ? m_wnd->getSwapchainImageCount() : 2;
        initInfo.CheckVkResultFn = nullptr;
        initInfo.PipelineInfoMain.MSAASamples = static_cast<VkSampleCountFlagBits>(ToVkSampleCount(m_wnd->getSwapchainSampleCount()));
        initInfo.PipelineInfoMain.RenderPass = static_cast<RHI::VK_PassGroup*>(m_passGroup)->getRenderPass();
        initInfo.PipelineInfoMain.Subpass = m_pass->index();
        initInfo.ApiVersion = VK_API_VERSION_1_0;
        ImGui_ImplVulkan_Init(&initInfo);

        m_sampler = static_cast<RHI::VK_Sampler*>(m_vkCtx->createResource<Sampler>());
        m_sampler->setLinear();
        m_sampler->create();
        m_platformInitialized = true;
    }

    void Win32_VK_ImguiContext::submitTick(RHI::CommandBuffer* cmdBuffer)
    {
        ZoneScoped();
        render();
        drawData(cmdBuffer);
    }

    void Win32_VK_ImguiContext::render()
    {
        ZoneScoped();
        ImGui::Render();
    }

    void Win32_VK_ImguiContext::drawData(RHI::CommandBuffer* cmdBuffer)
    {
        ZoneScoped();
        ImDrawData* drawData = ImGui::GetDrawData();
        VkCommandBuffer vkCmdBuffer = static_cast<RHI::VK_CommandBuffer*>(cmdBuffer)->commandBuffer();
        ImGui_ImplVulkan_RenderDrawData(drawData, vkCmdBuffer);
    }

    void Win32_VK_ImguiContext::destroyPlatform()
    {
        ZoneScoped();
        if (!m_platformInitialized)
        {
            return;
        }
        m_platformInitialized = false;

        if (m_wnd && m_wndProcCallbackId != 0)
        {
            m_wnd->removeWndProcCallback(m_wndProcCallbackId);
            m_wndProcCallbackId = 0;
        }

        for (auto& textureId : m_textureIds)
        {
            for (auto ds : textureId.second)
            {
                ImGui_ImplVulkan_RemoveTexture(ds);
            }
        }
        m_textureIds.clear();
        m_texturesHash.clear();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplWin32_Shutdown();

        if (m_sampler)
        {
            m_sampler->release();
            m_sampler = nullptr;
        }

        m_passGroup = nullptr;
    }
}
