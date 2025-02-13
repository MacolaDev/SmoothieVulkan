#version 450

//Vertex shader
#ifdef VERTEX_SHADER
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
#endif

//Fragment shader
#ifdef FRAGMENT_SHADER


layout (std140, binding = 0) uniform Standard
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
	vec3 viewPos;
    mat4 projectionViewMatrix;
};

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gMRAO;
uniform sampler2D gAlbedoSpec;

in vec2 TexCoords;

out vec4 FragColor;

float thickness = 0.1;

float calculateDepth(vec3 worldPos) {
    vec4 clipSpacePos = projectionViewMatrix * vec4(worldPos, 1.0);
    float ndcDepth = clipSpacePos.z / clipSpacePos.w;  // NDC depth in range [-1, 1]
    return (ndcDepth * 0.5) + 0.5;  // Map to [0, 1]
}

vec3 ScreenSpaceRayMarch(vec3 startPos, vec3 dir, float maxDist, int maxSteps) {
    float t = 0.0;
    for (int i = 0; i < maxSteps; i++) 
{
        vec3 pos = startPos + t * dir;
        vec4 projectedPos = projectionViewMatrix * vec4(pos, 1.0);
        projectedPos.xyz /= projectedPos.w; // Perspective division
        vec2 screenPos = projectedPos.xy * 0.5 + 0.5; // To screen space

        vec3 sampledPos = texture(gPosition, screenPos).rgb;
        float currentDepth = calculateDepth(sampledPos);
        
        float viewDepth = projectedPos.z;

        if (abs(currentDepth - viewDepth) < 0.001) 
        {
            return texture(gAlbedoSpec, screenPos).rgb;
        }

        t += 0.1; // Increase t to march further along the ray
        
        if (t > maxDist) break;
    }
    return vec3(0.0f); // No hit, fallback color
}


void main()
{   
    vec3 worldPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = texture(gNormal, TexCoords).rgb;
    
    vec3 viewDir = normalize(worldPos - viewPos);
    vec3 reflectionDir = reflect(viewDir, normal);

    float roughness = texture(gMRAO, TexCoords).g;
    
    float angleFalloff = max(dot(viewDir, reflectionDir), 0.0);
    
    //if(roughness >= 1.0f){discard;};

    vec3 reflected_color = ScreenSpaceRayMarch(worldPos, reflectionDir, 15.0, 50);
    vec3 finalColor = mix(reflected_color, vec3(0.0), roughness).xyz;
    
    FragColor = vec4(finalColor * (1 - roughness), 1);
}

#endif