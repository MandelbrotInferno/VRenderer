#version 460


#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_nonuniform_qualifier : require


#define MAX_UINT16 0xFFFF0000


layout(location = 0) flat in uint lv_materialHandle;
layout(location = 1) in vec2 lv_uv;

layout (location = 0) out vec4 lv_finalColor;

struct Material
{
	uint16_t m_baseColorMapHandle;
	uint16_t m_normalMapHandle;
	uint16_t m_emissiveMapHandle;
	uint16_t m_roughnessMetallicMapHandle;
};


layout (set = 0, binding = 1) uniform sampler2D Textures[];

layout(buffer_reference, std430, buffer_reference_align = 2) readonly buffer MaterialLayout{ 
	Material Materials[];
};

layout(push_constant) uniform PushConstantsLayout
{	
	uint64_t MaterialBufferPtr;
} PushConstants;

void main()
{
	MaterialLayout lv_materialLayout = MaterialLayout(PushConstants.MaterialBufferPtr);

	Material lv_currentMaterial = lv_materialLayout.Materials[lv_materialHandle];

	if(MAX_UINT16 == uint(lv_currentMaterial.m_baseColorMapHandle)) {
		lv_finalColor = vec4(0.0f);
	}
	else {
		lv_finalColor = texture(Textures[nonuniformEXT(uint(lv_currentMaterial.m_baseColorMapHandle))], lv_uv);
	}
}