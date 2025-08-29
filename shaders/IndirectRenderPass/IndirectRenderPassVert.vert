#version 460


#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) flat out uint lv_materialHandle;
layout(location = 1) out vec2 lv_uv;


struct Mesh
{
	uint m_firstIndexHandle;
	uint m_firstVertexHandle;
	uint m_totalNumIndices;
	uint m_materialHandle;
	uint m_modelTransformationHandle;
};

struct Vertex
{
	vec3 m_position;
	float m_uv_x;
	vec3 m_normal;
	float m_uv_y;
	vec4 m_tangent;
};

layout(std140, set = 0 ,binding = 0) uniform UniformBuffer
{
    mat4 Projection;
    mat4 View;
};


layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer MeshLayout{ 
	Mesh Meshes[];
};

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer VertexLayout{ 
	Vertex Vertices[];
};

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer ModelTransformationLayout{ 
	mat4 ModelTransformations[];
};

layout(push_constant) uniform PushConstantsLayout
{	
	uint64_t MeshBufferPtr;
	uint64_t VertexBufferPtr;
	uint64_t ModelTransformsPtr;
} PushConstants;



void main()
{
	MeshLayout lv_meshLayout = MeshLayout(PushConstants.MeshBufferPtr);
	VertexLayout lv_vertexLayout = VertexLayout(PushConstants.VertexBufferPtr);
	ModelTransformationLayout lv_modelTransformsLayout = ModelTransformationLayout(PushConstants.ModelTransformsPtr);
	
	Vertex lv_currentVertex = lv_vertexLayout.Vertices[gl_VertexIndex];
	Mesh lv_currentMesh = lv_meshLayout.Meshes[gl_BaseInstance];
	mat4 lv_currentModelTransform = lv_modelTransformsLayout.ModelTransformations[lv_currentMesh.m_modelTransformationHandle];

	lv_materialHandle = lv_currentMesh.m_materialHandle;
	lv_uv = vec2(lv_currentVertex.m_uv_x, lv_currentVertex.m_uv_y);
	gl_Position = Projection * View * lv_currentModelTransform * vec4(lv_currentVertex.m_position, 1.f);
}