#pragma once

float4 unpackNormals(uint packedNormal)
{
    float x = ((packedNormal >> 0) & 0xFF);
    float y = ((packedNormal >> 8) & 0xFF);
    float z = ((packedNormal >> 16) & 0xFF);
    float w = ((packedNormal >> 24) & 0xFF);

    float4 result = float4(x, y, z, w);

    result /= float4(127);
    result -= float4(1);

    return result;
}