



#include "GhazaEngine/VRenderer/VulkanSetLayoutAndPipelineLayoutGeneratorFromSPIRV.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSetLayoutFactory.hpp"
#include "GhazaEngine/VRenderer/VulkanResourceManager.hpp"
#include "GhazaEngine/VRenderer/VulkanWrappers/VulkanDescriptorSetAllocator.hpp"
#include "GhazaEngine/VRenderer/Utilities/Utilities.hpp"
#include <SPIRV-Reflect/spirv_reflect.h>
#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <fmt/core.h>
#include <algorithm>


namespace GhazaEngine
{
    namespace VRenderer
    {

        void VulkanSetLayoutAndPipelineLayoutGeneratorFromSPIRV::GenerateVulkanPipelineLayoutAndSetLayouts(VkDevice l_device, VulkanResourceManager& l_resManager, std::string_view l_renderpassSPIRVsFilePath, const std::pair<std::string, uint32_t>& l_bindlessTextureArraySizePair)
        {
            const std::filesystem::path lv_spirvCompiledShadersPath{ l_renderpassSPIRVsFilePath };
            std::vector<VkPushConstantRange> lv_ranges{};
            for (const auto& l_entry : std::filesystem::directory_iterator(lv_spirvCompiledShadersPath)) {

                const auto lv_fileSize = l_entry.file_size();
                auto lv_fullFilePath = l_entry.path().string();
                std::ifstream lv_spvFile(lv_fullFilePath, std::ios::ate | std::ios::binary);

                if (false == lv_spvFile.is_open()) {
                    printf("Failed to open %s\n", l_entry.path().string().c_str());
                    throw "Failed to open one of the spv files.\n";
                }
                std::vector<char> lv_bufferChar{};
                lv_bufferChar.resize(lv_fileSize);
                lv_spvFile.seekg(0);
                lv_spvFile.read(lv_bufferChar.data(), lv_fileSize);
                lv_spvFile.close();

                auto lv_extractedSetLayout = ExtractDescriptorSetLayoutsFromSPIRV_Bytecode(lv_bufferChar.data(), lv_bufferChar.size(), l_bindlessTextureArraySizePair);
                auto lv_range = ExtractPushConstBlockFromSPIRV_Bytecode(lv_bufferChar.data(), lv_bufferChar.size());
                if (0U != lv_range.size) {
                    lv_ranges.push_back(lv_range);
                }
                if (false == lv_extractedSetLayout.empty()) {
                    for (auto& l_prevSomeStageExtractedSetLayouts : m_extractedDescriptorSetLayouts) {
                        for (auto& l_prevExtractedSetLayout : l_prevSomeStageExtractedSetLayouts) {
                            for (auto& l_currentExtractedSetLayout : lv_extractedSetLayout) {
                                if (l_currentExtractedSetLayout.set_number == l_prevExtractedSetLayout.set_number) {

                                    for (const auto& lv_binding : l_currentExtractedSetLayout.bindings) {
                                        l_prevExtractedSetLayout.bindings.push_back(lv_binding);
                                    }

                                    l_prevExtractedSetLayout.create_info.pBindings = l_prevExtractedSetLayout.bindings.data();
                                    l_prevExtractedSetLayout.create_info.bindingCount = static_cast<uint32_t>(l_prevExtractedSetLayout.bindings.size());

                                    l_currentExtractedSetLayout.bindings.clear();
                                }
                            }
                        }
                    }

                    std::vector<VulkanSetLayoutAndPipelineLayoutGeneratorFromSPIRV::DescriptorSetLayout> lv_newExtractedSetLayouts{};
                    lv_newExtractedSetLayouts.reserve(lv_extractedSetLayout.size());

                    for (size_t i = 0U; i < lv_extractedSetLayout.size(); ++i) {
                        if (false == lv_extractedSetLayout[i].bindings.empty()) {
                            lv_newExtractedSetLayouts.emplace_back(std::move(lv_extractedSetLayout[i]));
                        }
                    }

                    if (false == lv_newExtractedSetLayouts.empty()) {
                        m_extractedDescriptorSetLayouts.emplace_back(std::move(lv_newExtractedSetLayouts));
                    }
                }
            }

            size_t lv_totalLayouts{};
            for (auto& l_extractedSetLayouts : m_extractedDescriptorSetLayouts) {
                lv_totalLayouts += l_extractedSetLayouts.size();
            }
            std::vector<VkDescriptorSetLayout> lv_vkSetLayouts{};
            lv_vkSetLayouts.reserve(lv_totalLayouts);
            VulkanDescriptorSetLayoutFactory lv_vkSetLayoutFactory{};
            std::string lv_spvFilePath{ l_renderpassSPIRVsFilePath };
            auto lv_firstOccurence = lv_spvFilePath.find_first_of("\\");
            auto lv_lastOccurence = lv_spvFilePath.find_last_of("\\");
            auto lv_baseName = lv_spvFilePath.substr(lv_firstOccurence + 1U, lv_lastOccurence - lv_firstOccurence - 1U);
            auto lv_baseNameSetLayouts = lv_baseName + "{}";

            for (const auto& l_extractedSetLayoutsOfSomeStage : m_extractedDescriptorSetLayouts) {
                for (const auto& l_extractedSetLayout : l_extractedSetLayoutsOfSomeStage) {
                    for (const auto& l_binding : l_extractedSetLayout.bindings) {
                        lv_vkSetLayoutFactory.AddBinding(l_binding.binding, l_binding.descriptorType, l_binding.descriptorCount, l_binding.stageFlags);
                    }
                    auto lv_lastElement = lv_vkSetLayouts.emplace_back(lv_vkSetLayoutFactory.GenerateSetLayout(l_device));
                    lv_vkSetLayoutFactory.Reset();
                    l_resManager.AddVulkanSetLayout(std::move(fmt::vformat(lv_baseNameSetLayouts, fmt::make_format_args(l_extractedSetLayout.set_number))), lv_lastElement);
                }

            }

            auto Compare = [](const VkPushConstantRange& l_a, const VkPushConstantRange& l_b) ->bool
                {
                    if (l_a.stageFlags < l_b.stageFlags) {
                        return true;
                    }
                    else {
                        return false;
                    }
                };
            std::sort(lv_ranges.begin(), lv_ranges.end(), Compare);

            auto lv_vulkanPipelineLayout = Utilities::GenerateVkPipelineLayout(l_device, lv_vkSetLayouts, lv_ranges);
            l_resManager.AddVulkanPipelineLayout(std::move(lv_baseName), lv_vulkanPipelineLayout);

            m_extractedDescriptorSetLayouts.clear();
        }


        std::vector<VulkanSetLayoutAndPipelineLayoutGeneratorFromSPIRV::DescriptorSetLayout> VulkanSetLayoutAndPipelineLayoutGeneratorFromSPIRV::ExtractDescriptorSetLayoutsFromSPIRV_Bytecode(const void* l_binary, const size_t l_sizeOfBinary, const std::pair<std::string, uint32_t>& l_bindlessTextureArraySizePair)
        {
            SpvReflectShaderModule lv_module = {};
            SpvReflectResult lv_result = spvReflectCreateShaderModule(l_sizeOfBinary, l_binary, &lv_module);
            assert(lv_result == SPV_REFLECT_RESULT_SUCCESS);

            uint32_t lv_count = 0;
            lv_result = spvReflectEnumerateDescriptorSets(&lv_module, &lv_count, NULL);
            assert(lv_result == SPV_REFLECT_RESULT_SUCCESS);

            std::vector<SpvReflectDescriptorSet*> lv_sets(lv_count);
            lv_result = spvReflectEnumerateDescriptorSets(&lv_module, &lv_count, lv_sets.data());
            assert(lv_result == SPV_REFLECT_RESULT_SUCCESS);


            std::vector<DescriptorSetLayout> lv_set_layouts(lv_sets.size(), DescriptorSetLayout{});
            for (size_t i_set = 0; i_set < lv_sets.size(); ++i_set) {
                const SpvReflectDescriptorSet& lv_refl_set = *(lv_sets[i_set]);
                DescriptorSetLayout& lv_layout = lv_set_layouts[i_set];

                if (SpvOpTypeRuntimeArray != (*lv_refl_set.bindings)->type_description->op) {
                    lv_layout.bindings.resize(lv_refl_set.binding_count);
                }
                else {
                    if (l_bindlessTextureArraySizePair.first == std::string((*lv_refl_set.bindings)->name)) {
                        lv_layout.bindings.resize(l_bindlessTextureArraySizePair.second);
                        const SpvReflectDescriptorBinding& refl_binding = *(lv_refl_set.bindings[0]);

                        for (uint32_t i_binding = 0; i_binding < l_bindlessTextureArraySizePair.second; ++i_binding) {
                            VkDescriptorSetLayoutBinding& layout_binding = lv_layout.bindings[i_binding];
                            layout_binding.binding = refl_binding.binding + i_binding;
                            layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
                            layout_binding.descriptorCount = 1;
                            layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(lv_module.shader_stage);
                        }
                        lv_layout.set_number = lv_refl_set.set;
                        lv_layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                        lv_layout.create_info.bindingCount = (uint32_t)l_bindlessTextureArraySizePair.second;
                        lv_layout.create_info.pBindings = lv_layout.bindings.data();
                    }

                    continue;
                }
                for (uint32_t i_binding = 0; i_binding < lv_refl_set.binding_count; ++i_binding) {
                    const SpvReflectDescriptorBinding& refl_binding = *(lv_refl_set.bindings[i_binding]);
                    VkDescriptorSetLayoutBinding& layout_binding = lv_layout.bindings[i_binding];
                    layout_binding.binding = refl_binding.binding;
                    layout_binding.descriptorType = static_cast<VkDescriptorType>(refl_binding.descriptor_type);
                    layout_binding.descriptorCount = 1;
                    for (uint32_t i_dim = 0; i_dim < refl_binding.array.dims_count; ++i_dim) {
                        layout_binding.descriptorCount *= refl_binding.array.dims[i_dim];
                    }
                    layout_binding.stageFlags = static_cast<VkShaderStageFlagBits>(lv_module.shader_stage);
                }
                lv_layout.set_number = lv_refl_set.set;
                lv_layout.create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                lv_layout.create_info.bindingCount = lv_refl_set.binding_count;
                lv_layout.create_info.pBindings = lv_layout.bindings.data();
            }

            spvReflectDestroyShaderModule(&lv_module);
            return lv_set_layouts;

        }

        VkPushConstantRange VulkanSetLayoutAndPipelineLayoutGeneratorFromSPIRV::ExtractPushConstBlockFromSPIRV_Bytecode(const void* l_binary, const size_t l_sizeOfBinary)
        {
            uint32_t lv_count{};
            SpvReflectShaderModule lv_module{};
            SpvReflectResult lv_result = spvReflectCreateShaderModule(l_sizeOfBinary, l_binary, &lv_module);
            assert(lv_result == SPV_REFLECT_RESULT_SUCCESS);
            spvReflectEnumeratePushConstantBlocks(&lv_module, &lv_count, NULL);
            std::vector<SpvReflectBlockVariable*> lv_push_constant(lv_count);
            lv_result = spvReflectEnumeratePushConstantBlocks(&lv_module, &lv_count, lv_push_constant.data());
            assert(lv_result == SPV_REFLECT_RESULT_SUCCESS);

            VkPushConstantRange lv_range{};

            if (false == lv_push_constant.empty()) {
                lv_range.offset = lv_push_constant[0]->offset;
                lv_range.size = lv_push_constant[0]->size;
                lv_range.stageFlags = lv_module.shader_stage;
            }


            spvReflectDestroyShaderModule(&lv_module);
            return lv_range;
        }
    }
}
