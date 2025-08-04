cmake_minimum_required(VERSION 3.21)
project(VRenderer LANGUAGES CXX)

# C++20 Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# VCPKG Toolchain Integration
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Force Release build unless overridden
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Dependencies
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(stb CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(vk-bootstrap CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(tracy CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

# Gather all source/header files recursively
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.c
    include/*.h
    include/*.hpp
)

# Executable
add_executable(VRenderer ${SOURCES})

# Link libraries
target_link_libraries(VRenderer PRIVATE
    glm::glm
    fmt::fmt
    stb::stb
    fastgltf::fastgltf
    vk_bootstrap::vk_bootstrap
    volk::volk
    Vulkan::Vulkan
    VulkanMemoryAllocator::VulkanMemoryAllocator
    tracy::tracy
    SDL3::SDL3
)

# Include directories
target_include_directories(VRenderer PRIVATE include)

# Define VOLK_IMPLEMENTATION in one translation unit
target_compile_definitions(MyApp PRIVATE VOLK_IMPLEMENTATION)

# Setup Visual Studio Filters (Virtual Folders)
if(MSVC)
    foreach(FILE IN LISTS SOURCES)
        # Get relative path from project root
        file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}" "${FILE}")
        
        # Extract directory and replace '/' with '\'
        get_filename_component(FOLDER "${REL_PATH}" PATH)
        string(REPLACE "/" "\\" FOLDER "${FOLDER}")

        source_group("${FOLDER}" FILES "${FILE}")
    endforeach()
endif()

# Platform Specific Messages (Optional)
if(WIN32)
    message(STATUS "Configuring for Windows")
elseif(UNIX)
    message(STATUS "Configuring for Linux")
endif()
