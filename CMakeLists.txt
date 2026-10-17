include(platform/platform.cmake)
include(config/set_utf8_compiler_encoding.cmake)

# ImGui核心源文件 - 直接使用当前目录下的imgui源码
set(IMGUI_SOURCES
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_vulkan.cpp
)

if (NOT FCT_ANDROID)
    list(APPEND IMGUI_SOURCES imgui/backends/imgui_impl_glfw.cpp)
endif()

if (FCT_WIN32)
    list(APPEND IMGUI_SOURCES
            imgui/backends/imgui_impl_win32.cpp
            imgui/backends/imgui_impl_dx12.cpp
    )
endif()

set(IMNODES_SOURCES
        imnodes/imnodes.cpp
)

file(GLOB FCT_IMGUI_HEADERS
        LIST_DIRECTORIES false
        CONFIGURE_DEPENDS
        "./src/*.h"
        "./src/*.hpp"
)

list(FILTER FCT_IMGUI_HEADERS EXCLUDE REGEX ".*FCT_IMGUI\\.h$")

add_library(FCT_IMGUI STATIC
        src/ImguiContext.cpp
        src/headers.cpp
        ${IMGUI_SOURCES}
        ${IMNODES_SOURCES}
)
if (NOT FCT_ANDROID)
    target_sources(FCT_IMGUI PRIVATE src/GLFW_VK_ImguiContext.cpp)
endif()
if (FCT_WIN32)
    target_sources(FCT_IMGUI PRIVATE
            src/Win32_D3D12_ImguiContext.cpp
            src/Win32_VK_ImguiContext.cpp
            src/GLFW_D3D12_ImguiContext.cpp
    )
endif()
set_utf8_for_target(FCT_IMGUI)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/FCT_IMGUI)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/FCT_IMGUI.h")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/FCT_IMGUI.h ${CMAKE_CURRENT_SOURCE_DIR}/include/FCT_IMGUI.h COPYONLY)
endif()


foreach(header_file ${FCT_IMGUI_HEADERS})
    file(RELATIVE_PATH relative_path ${CMAKE_CURRENT_SOURCE_DIR}/src ${header_file})
    get_filename_component(header_dir ${relative_path} DIRECTORY)

    if(header_dir)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/FCT_IMGUI/${header_dir})
    endif()

    configure_file(${header_file} ${CMAKE_CURRENT_SOURCE_DIR}/include/FCT_IMGUI/${relative_path} COPYONLY)
endforeach()

target_include_directories(FCT_IMGUI PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/imnodes
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if (FCT_ANDROID)
    find_package(VulkanHeaders CONFIG REQUIRED)
    target_link_libraries(FCT_IMGUI PRIVATE FCT
            Vulkan::Headers
            vulkan
    )
else()
    find_package(Vulkan REQUIRED)
    target_link_libraries(FCT_IMGUI PRIVATE FCT
            ${Vulkan_LIBRARIES}
    )
    if (FCT_WIN32)
        target_link_libraries(FCT_IMGUI PRIVATE
                d3d12
                dxgi
                d3dcompiler
                dwmapi
        )
    endif()
endif()

function(target_link_fct_imgui_libraries target_name)
    target_link_libraries(${target_name} FCT_IMGUI)
endfunction()