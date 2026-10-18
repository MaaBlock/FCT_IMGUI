#include "./Win32_D3D12_ImguiContext.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace FCT
{
    Win32_D3D12_ImguiContext::Win32_D3D12_ImguiContext(Win32_Window* wnd, D3D12_Context* ctx)
        : ImguiContext(ctx)
        , m_passGroup(nullptr)
        , m_wnd(wnd)
        , m_d3dCtx(ctx)
        , m_currentPush(0)
        , m_currentShow(1)
        , m_wndProcCallbackId(0)
    {
        m_ctx = ctx;
    }

    Win32_D3D12_ImguiContext::~Win32_D3D12_ImguiContext()
    {
        destroyPlatform();
        m_textures.clear();
    }

    Win32_D3D12_ImguiContext::SrvDescriptorPair Win32_D3D12_ImguiContext::allocSrvDescriptor()
    {
        SrvDescriptorPair pair{};
        if (m_freeDescriptors.pop(pair))
        {
            return pair;
        }

        auto* descPool = static_cast<RHI::D3D12_DescriptorPool*>(m_d3dCtx->getDescriptorPool());
        if (descPool)
        {
            uint32_t baseIndex = 0;
            descPool->allocCbvSrvUavRange(1, pair.cpuHandle, pair.gpuHandle, baseIndex);
        }
        return pair;
    }

    void Win32_D3D12_ImguiContext::freeSrvDescriptor(const SrvDescriptorPair& pair)
    {
        if (pair.cpuHandle.ptr == 0)
        {
            return;
        }
        m_freeDescriptors.push(pair);
    }

    DXGI_FORMAT Win32_D3D12_ImguiContext::resolveRtvFormat() const
    {
        if (m_pass && !m_pass->renderTargets().empty())
        {
            Image* target = m_pass->renderTargets().begin()->second;
            if (target && target->format() != Format::UNDEFINED)
            {
                return ToDxgiFormat(target->format());
            }
        }
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    }

    DXGI_FORMAT Win32_D3D12_ImguiContext::resolveDsvFormat() const
    {
        if (m_pass && m_pass->depthStencil())
        {
            Format fmt = m_pass->depthStencil()->format();
            if (fmt != Format::UNDEFINED)
            {
                return ToDxgiFormat(fmt);
            }
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    void Win32_D3D12_ImguiContext::srvDescriptorAllocCallback(
        ImGui_ImplDX12_InitInfo* info,
        D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
    {
        auto* self = static_cast<Win32_D3D12_ImguiContext*>(info->UserData);
        SrvDescriptorPair pair = self->allocSrvDescriptor();
        *outCpuHandle = pair.cpuHandle;
        *outGpuHandle = pair.gpuHandle;
    }

    void Win32_D3D12_ImguiContext::srvDescriptorFreeCallback(
        ImGui_ImplDX12_InitInfo* info,
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
    {
        auto* self = static_cast<Win32_D3D12_ImguiContext*>(info->UserData);
        self->freeSrvDescriptor({ cpuHandle, gpuHandle });
    }

    void Win32_D3D12_ImguiContext::addTexture(std::string name, MutilBufferImage* image)
    {
        m_textures[name] = image;
        if (!m_platformInitialized)
        {
            return;
        }
        std::vector<RHI::TextureView*> rhiTextureViews = image->tvs();
        for (uint32_t i = 0; i < image->imageCount() && i < rhiTextureViews.size(); ++i)
        {
            auto* tv = static_cast<RHI::D3D12_TextureView*>(rhiTextureViews[i]);
            SrvDescriptorPair pair = allocSrvDescriptor();
            if (tv && tv->isValid() && pair.cpuHandle.ptr != 0)
            {
                m_d3dCtx->getDevice()->CopyDescriptorsSimple(
                    1,
                    pair.cpuHandle,
                    tv->getCpuHandle(),
                    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            }
            m_textureIds[name].push_back(pair);
        }
    }

    void Win32_D3D12_ImguiContext::addTexture(std::string name, SingleBufferImage* image)
    {
        m_textures[name] = image;
        if (!m_platformInitialized)
        {
            return;
        }
        auto* tv = static_cast<RHI::D3D12_TextureView*>(image->currentTextureView());
        SrvDescriptorPair pair = allocSrvDescriptor();
        if (tv && tv->isValid() && pair.cpuHandle.ptr != 0)
        {
            m_d3dCtx->getDevice()->CopyDescriptorsSimple(
                1,
                pair.cpuHandle,
                tv->getCpuHandle(),
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        m_textureIds[name].push_back(pair);
    }

    void Win32_D3D12_ImguiContext::addTexture(std::string name, Image* image)
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

    void Win32_D3D12_ImguiContext::removeTexture(std::string name)
    {
        auto it = m_textureIds.find(name);
        if (it != m_textureIds.end())
        {
            for (const auto& pair : it->second)
            {
                freeSrvDescriptor(pair);
            }
            m_textureIds.erase(it);
        }
        m_textures.erase(name);
    }

    ImTextureID Win32_D3D12_ImguiContext::getTexture(std::string name)
    {
        auto it = m_textureIds.find(name);
        if (it != m_textureIds.end() && !it->second.empty())
        {
            return static_cast<ImTextureID>(it->second[0].gpuHandle.ptr);
        }
        return static_cast<ImTextureID>(0);
    }

    void Win32_D3D12_ImguiContext::updateTexture(std::string name)
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

    void Win32_D3D12_ImguiContext::push(UIDeclare uiFunc)
    {
        m_uiDeclares[m_currentPush].push(uiFunc);
    }

    void Win32_D3D12_ImguiContext::swapBuffer()
    {
        std::swap(m_currentPush, m_currentShow);
    }

    void Win32_D3D12_ImguiContext::submit(RHI::CommandBuffer* cmdBuf)
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

    void Win32_D3D12_ImguiContext::newFrame()
    {
        ZoneScoped();
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void Win32_D3D12_ImguiContext::updatePassResource()
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

    void Win32_D3D12_ImguiContext::createPlatform()
    {
        ZoneScoped();
        m_passGroup = m_pass ? m_pass->group() : nullptr;

        ImGui_ImplWin32_Init(m_wnd->getHwnd());
        m_wndProcCallbackId = m_wnd->addWndProcCallback([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
        {
            return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
        });

        auto* descPool = static_cast<RHI::D3D12_DescriptorPool*>(m_d3dCtx->getDescriptorPool());
        int swapchainCount = m_wnd->getSwapchainImageCount();

        ImGui_ImplDX12_InitInfo initInfo{};
        initInfo.Device = m_d3dCtx->getDevice();
        initInfo.CommandQueue = m_d3dCtx->getGraphicsQueue();
        initInfo.NumFramesInFlight = swapchainCount > 0 ? swapchainCount : static_cast<int>(m_d3dCtx->maxFrameInFlight());
        initInfo.RTVFormat = resolveRtvFormat();
        initInfo.DSVFormat = resolveDsvFormat();
        initInfo.UserData = this;
        initInfo.SrvDescriptorHeap = descPool ? descPool->getCbvSrvUavHeap() : nullptr;
        initInfo.SrvDescriptorAllocFn = &Win32_D3D12_ImguiContext::srvDescriptorAllocCallback;
        initInfo.SrvDescriptorFreeFn = &Win32_D3D12_ImguiContext::srvDescriptorFreeCallback;

        ImGui_ImplDX12_Init(&initInfo);
        m_platformInitialized = true;
    }

    void Win32_D3D12_ImguiContext::submitTick(RHI::CommandBuffer* cmdBuffer)
    {
        ZoneScoped();
        render();
        drawData(cmdBuffer);
    }

    void Win32_D3D12_ImguiContext::render()
    {
        ZoneScoped();
        ImGui::Render();
    }

    void Win32_D3D12_ImguiContext::drawData(RHI::CommandBuffer* cmdBuffer)
    {
        ZoneScoped();
        ImDrawData* drawData = ImGui::GetDrawData();
        auto* d3dCmdBuf = static_cast<RHI::D3D12_CommandBuffer*>(cmdBuffer);
        ID3D12GraphicsCommandList* cmdList = d3dCmdBuf->commandList();

        auto* descPool = static_cast<RHI::D3D12_DescriptorPool*>(m_d3dCtx->getDescriptorPool());
        if (descPool)
        {
            ID3D12DescriptorHeap* heaps[] = { descPool->getCbvSrvUavHeap(), descPool->getSamplerHeap() };
            uint32_t heapCount = (heaps[0] && heaps[1]) ? 2 : (heaps[0] ? 1 : 0);
            if (heapCount > 0)
            {
                cmdList->SetDescriptorHeaps(heapCount, heaps);
            }
        }

        ImGui_ImplDX12_RenderDrawData(drawData, cmdList);
    }

    void Win32_D3D12_ImguiContext::destroyPlatform()
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

        for (const auto& entry : m_textureIds)
        {
            for (const auto& pair : entry.second)
            {
                freeSrvDescriptor(pair);
            }
        }
        m_textureIds.clear();
        m_texturesHash.clear();

        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();

        m_passGroup = nullptr;
    }
}
