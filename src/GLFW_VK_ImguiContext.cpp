#include "./GLFW_VK_ImguiContext.h"

namespace FCT
{
    GLFW_VK_ImguiContext::GLFW_VK_ImguiContext(GLFW_Window* wnd, VK_Context* ctx) : ImguiContext(ctx)
    {
        m_wnd = wnd;
        m_vkCtx = ctx;
        m_currentPush = 0;
        m_currentShow = 1;
        m_ctx = ctx;
        // The ImGui_ImplGlfw_NewFrame() will handle input updates, so we don't need to call newFrame_updateInput() explicitly.
        // However, if there are custom input handling or event dispatching that needs to happen on the main thread,
        // it should be integrated into the GLFW_Window's event loop or dispatched via sendUiTask.
        /*
        m_wnd->postTicker([this]()
        {
            if (m_platformInitialized)
                newFrame_updateInput();
        });
        */
        /*
        m_wnd->getCallBack()->addFileDropCallback([this](Window*, const std::vector<std::string>& paths)
        {
            if (m_platformInitialized)
            {

            }
        });*/
        /*
        m_wnd->postTicker([this]()
        {
            newFrame_updateInput();
        });*/
    }

    GLFW_VK_ImguiContext::~GLFW_VK_ImguiContext()
    {
        for (auto img : m_textures)
        {
            removeTexture(img.first);
        }
    }

    void GLFW_VK_ImguiContext::addTexture(std::string name, MutilBufferImage* image)
    {
        m_textures[name] = image;
        std::vector<RHI::TextureView*> rhiTextureViews = image->tvs();
        for (uint32_t i = 0; i < image->imageCount(); ++i)
        {
            RHI::VK_TextureView* tv = static_cast<RHI::VK_TextureView*>(rhiTextureViews[i]);
            vk::ImageView imageViewVk = tv->view();
            vk::Sampler samplerVk = m_sampler->getSampler();
            VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(
                samplerVk, imageViewVk, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            m_textureIds[name].push_back(descriptorSet);
        }
    }

    void GLFW_VK_ImguiContext::addTexture(std::string name, SingleBufferImage* image)
    {
        m_textures[name] = image;
        if (m_platformInitialized)
        {
            image->currentTextureView();
            RHI::VK_TextureView* tv = static_cast<RHI::VK_TextureView*>(image->currentTextureView());
            vk::ImageView imageViewVk = tv->view();
            vk::Sampler samplerVk = m_sampler->getSampler();
            VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(
                samplerVk, imageViewVk, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            m_textureIds[name].push_back(descriptorSet);
        }
    }

    void GLFW_VK_ImguiContext::addTexture(std::string name, Image* image)
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

    void GLFW_VK_ImguiContext::removeTexture(std::string name)
    {
        for (auto ds : m_textureIds[name])
        {
            ImGui_ImplVulkan_RemoveTexture(ds);
        }
        m_textures.erase(name);
        m_textureIds.erase(name);
    }

    ImTextureID GLFW_VK_ImguiContext::getTexture(std::string name)
    {
        auto it = m_textureIds.find(name);
        if (it != m_textureIds.end())
        {
            return reinterpret_cast<ImTextureID>(it->second[0]);
        }
        return reinterpret_cast<ImTextureID>(nullptr);
    }

    void GLFW_VK_ImguiContext::updateTexture(std::string name)
    {
        auto image = m_textures[name];
        removeTexture(name);
        addTexture(name, image);
    }

    void GLFW_VK_ImguiContext::push(UIDeclare uiFunc)
    {
        m_uiDeclares[m_currentPush].push(uiFunc);
    }

    void GLFW_VK_ImguiContext::swapBuffer()
    {
        std::swap(m_currentPush, m_currentShow);
    }

    void GLFW_VK_ImguiContext::submit(RHI::CommandBuffer* cmdBuf)
    {
        newFrame();
        while (!m_uiDeclares[m_currentShow].empty())
        {
            auto uiFunc = m_uiDeclares[m_currentShow].front();
            uiFunc();
            m_uiDeclares[m_currentShow].pop();
        }
        render();
        drawData(cmdBuf);
    }

    void GLFW_VK_ImguiContext::newFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void GLFW_VK_ImguiContext::updatePassResource()
    {
        if (!m_platformInitialized)
            return;
        std::vector<std::string> needUpdate = {};
        for (auto it : m_textures)
        {
            auto hash = it.second->textureViewHash();
            if (m_texturesHash[it.first] != hash)
            {
                m_texturesHash[it.first] = hash;
                needUpdate.push_back(it.first);
            }
        }
        for (auto name : needUpdate)
        {
            updateTexture(name);
        }
    }

    void GLFW_VK_ImguiContext::createPlatform()
    {
        m_passGroup = m_pass->group();
        ImGui_ImplGlfw_InitForVulkan(m_wnd->getWindow(), true);
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
        initInfo.ImageCount = m_wnd->getSwapchainImageCount();
        initInfo.CheckVkResultFn = nullptr;
        initInfo.PipelineInfoMain.MSAASamples = static_cast<VkSampleCountFlagBits>(ToVkSampleCount(m_wnd->getSwapchainSampleCount()));
        initInfo.PipelineInfoMain.RenderPass = static_cast<RHI::VK_PassGroup*>(m_passGroup)->getRenderPass();
        initInfo.PipelineInfoMain.Subpass = m_pass->index();
        initInfo.ApiVersion = VK_API_VERSION_1_0; // Explicitly set API version
        ImGui_ImplVulkan_Init(&initInfo);
        m_sampler = static_cast<RHI::VK_Sampler*>(m_vkCtx->createResource<Sampler>());
        m_sampler->setLinear();
        m_sampler->create();
        m_platformInitialized = true;
    }


    void GLFW_VK_ImguiContext::submitTick(RHI::CommandBuffer* cmdBuffer)
    {
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();

        VkCommandBuffer vkCmdBuffer = static_cast<RHI::VK_CommandBuffer*>(cmdBuffer)->commandBuffer();
        ImGui_ImplVulkan_RenderDrawData(drawData, vkCmdBuffer);
    }

    void GLFW_VK_ImguiContext::render()
    {
        ImGui::Render();
    }

    void GLFW_VK_ImguiContext::drawData(RHI::CommandBuffer* cmdBuffer)
    {
        ImDrawData* drawData = ImGui::GetDrawData();

        VkCommandBuffer vkCmdBuffer = static_cast<RHI::VK_CommandBuffer*>(cmdBuffer)->commandBuffer();
        ImGui_ImplVulkan_RenderDrawData(drawData, vkCmdBuffer);
    }


    void GLFW_VK_ImguiContext::destroyPlatform()
    {
        if (!m_platformInitialized)
            return;
        m_platformInitialized = false;

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

        ImGui_ImplGlfw_Shutdown();

        if (m_sampler)
        {
            m_sampler->release();
            m_sampler = nullptr;
        }

        m_passGroup = nullptr;
    }
}
