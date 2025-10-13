#version 450

#ifdef SSHADER_COMPUTE_SWIZZLER
layout(local_size_x = 1, local_size_y = 1) in;

layout(set = 0, binding = 0) uniform sampler2D srcImage;
layout(set = 0, binding = 1, rgba8) uniform writeonly image2D dstImage;

layout(push_constant, std430) uniform pc
{
    mat4 swizzlerMatrix;
};


void main()
{

    //Get UV coordinates
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(dstImage);
    vec2 uv = vec2(coord) / vec2(size);
    uv.y = 1.0f - uv.y;
    
    vec4 source = texture(srcImage, uv).rgba;
    vec4 result = swizzlerMatrix * source;
    result.xyz = clamp(result.xyz, vec3(0.0f), vec3(1.0f));
    imageStore(dstImage, coord, vec4(result.xyz, 1.0f));

}

#endif