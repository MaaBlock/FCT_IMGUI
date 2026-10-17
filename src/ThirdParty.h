//
// Created by Administrator on 2025/4/26.
//

#ifndef THIRDPARTY_H
#define THIRDPARTY_H
#include <FCT/headers.h>
#include <FCT/FCTAPI.h>
#include <imgui.h>
#ifdef FCT_USE_VULKAN
#include <imgui_impl_vulkan.h>
#endif
#ifdef FCT_USE_D3D12
#include <imgui_impl_dx12.h>
#endif
#ifdef FCT_USE_GLFW
#include <imgui_impl_glfw.h>
#endif
#ifdef FCT_WIN32
#include <imgui_impl_win32.h>
#endif
#endif //THIRDPARTY_H

