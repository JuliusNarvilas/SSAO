#version 330 core

out float FragColor;

in mat4 inverseProj;
in mat4 inverseProjView;
in mat4 vpMatrix;

uniform sampler2D depthTex;
uniform sampler2D normalTex;
uniform sampler2D noiseTex;

uniform int kernelSize;
uniform vec2 pixelSize;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

uniform vec3 samples[64];

// tile noise texture over screen based on screen dimensions divided by noise size
//const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); // screen = 800x600

void main()
{
    vec2 noiseScale = vec2(1.0f) / pixelSize / vec2(4.0f);
    
    vec3 fragScreenPos = vec3(( gl_FragCoord.x * pixelSize.x), (gl_FragCoord.y * pixelSize.y), 0.0f);
    fragScreenPos.z = texture(depthTex , fragScreenPos.xy).r;
    
    vec3 normal = normalize(texture(normalTex, fragScreenPos.xy).xyz * 2.0f - 1.0f);
    vec3 randomVec = texture(noiseTex, fragScreenPos.xy * noiseScale).xyz * 2.0f - 1.0f;
    
    fragScreenPos = fragScreenPos * 2.0f - 1.0f;
    
    vec4 clip = inverseProjView * vec4(fragScreenPos, 1.0f);
    vec3 fragPos = clip.xyz / clip.w;
    vec4 fragViewPos = viewMatrix * vec4(fragPos, 1.0f);
    
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    mat3 TBN = mat3(tangent, cross(normal, tangent), normal);
    
    const float radius = 1.0f;
    float occlusion = 0.0f;
    
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i] * radius + fragPos; // From tangent to view-space
        
        vec4 samplePosVec4 = vec4(samplePos, 1.0f);
        float expectedSampleDepth = (viewMatrix * samplePosVec4).z;
        
        //turn sample to screen space
        vec4 bufferSample = vpMatrix * samplePosVec4; // from view to clip-space
        bufferSample /= bufferSample.w; // perspective divide
       
        //new sample depth based on depth texture
        vec2 normalizedScreenSample = bufferSample.xy * 0.5f + 0.5f; // transform to range 0.0 - 1.0
        bufferSample.z = texture(depthTex , normalizedScreenSample).r;
        bufferSample = inverseProj * vec4(bufferSample.xyz * 2.0f - 1.0f, 1.0f); //from clip-space to view
        bufferSample /= bufferSample.w;
        
        //prevent far background objects interacting with close foreground objects
        float rangeCheck = smoothstep(0.3f, 1.0f, radius / abs(fragViewPos.z - bufferSample.z));
        
        occlusion += (bufferSample.z >= expectedSampleDepth ? 1.0f : 0.0f) * rangeCheck;
    }
    
    FragColor = 1.0f - (occlusion / float(kernelSize));
}