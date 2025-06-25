#pragma once


#include <fmt/core.h>
#include <vulkan/vk_enum_string_helper.h>


namespace VRenderer
{
#define VULKAN_CHECK(x)                                                     \
        VkResult err = x;                                               \
        if (err) {                                                      \
            fmt::print("Detected Vulkan error at line: {0}\nFile: {1}\nVulkan Error: {3}\n",__LINE__, __FILE__, string_VkResult(err),); \
            throw "Vulkan error occured. Aborting by throwing exception.";                                                    \
        }
}
