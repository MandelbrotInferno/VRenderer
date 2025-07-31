#pragma once


#include <fmt/core.h>
#include <volk.h>


namespace VRenderer
{
    const char* string_VkResult(VkResult input_value);
#define VULKAN_CHECK(x)                                                     \
        {\
    VkResult err = x;                                               \
        if (err) {\
                fmt::print("Detected Vulkan error at line: {0}\nFile: {1}\nVulkan Error: {2}\n", __LINE__, __FILE__, string_VkResult(err)); \
                throw "Vulkan error occured. Aborting by throwing exception.";                                                    \
        }\
    }
}
