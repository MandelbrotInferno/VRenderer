#pragma once

#include "include/VulkanFoundationalElements.hpp"
#include "include/VulkanSwapchain.hpp"
#include "include/VulkanQueueGraphics.hpp"
#include "include/VulkanCommandbufferReset.hpp"
#include <array>

struct SDL_Window;

namespace VRenderer
{
	class Renderer final
	{
	public:
		
		VulkanCommandbufferReset& GetCurrentFrameGraphicsCmdBuffer();

		void Init(SDL_Window* l_window);

		void CleanUp();

	private:

		void InitializeVulkanFoundationalElementsAndGraphicsQueue(SDL_Window* l_window);
		void InitializeVulkanSwapchain(SDL_Window* l_window);
		void InitializeVulkanGraphicsCommandbuffers();

	public:
		
		VulkanFoundationalElements m_vulkanFoundational{};
		VulkanSwapchain m_vulkanSwapchain{};
		VulkanQueueGraphics m_vulkanQueue{};
		VkDevice m_device{};

	private:
		static constexpr uint32_t m_maxCommandBuffers{ 2U };
		uint32_t m_currentGraphicsCmdBufferIndex{};
		std::array<VulkanCommandbufferReset, m_maxCommandBuffers> m_vulkanGraphicsCmdBuffers{};
	};
}