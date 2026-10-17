//
// Created by Administrator on 2025/4/26.
//

#ifndef FCT_IMGUIAPI_H
#define FCT_IMGUIAPI_H
#include "./headers.h"
#ifdef FCT_USE_GLFW
#ifdef FCT_USE_VULKAN
#include "./GLFW_VK_ImguiContext.h"
#endif
#ifdef FCT_USE_D3D12
#include "./GLFW_D3D12_ImguiContext.h"
#endif
#endif
#ifdef FCT_WIN32
#ifdef FCT_USE_D3D12
#include "./Win32_D3D12_ImguiContext.h"
#endif
#ifdef FCT_USE_VULKAN
#include "./Win32_VK_ImguiContext.h"
#endif
#endif
#endif //FCTAPI_H
