#version 450
#extension GL_ARB_shading_language_include: enable 

#ifdef SSHADER_COMPUTE_GPU_SELECTED_DATA

#include "../ShadersGeneral.glslh"
#define DESCRIPTOR_CAMERA MAKE_DESCRIPTOR_SET(0, 0)
#include "../Descriptors.glslh"
layout(local_size_x = 1, local_size_y = 1) in;

layout(push_constant, std430) uniform pc_in
{
    vec2 uv;
};

struct gpu_output
{
    vec3 world_pos;
    float depth; 
};

layout(set = 1, binding = 0, std430) buffer writeonly pc_out 
{
    gpu_output _output;
};
layout(set = 1, binding = 1) uniform sampler2D depthImage;


void main()
{
    float depth = texture(depthImage, uv).r;
    vec4 world_pos = Camera.invProjectionViewMatrix * vec4(vec2(uv) * 2.0f - 1.0f, depth, 1.0f);
    world_pos.xyzw /= world_pos.w;

    _output.world_pos = world_pos.xyz;
    _output.depth = depth;
}

#endif