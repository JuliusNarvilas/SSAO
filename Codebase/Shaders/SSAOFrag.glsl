#version 330 core

out float FragColor;

in mat4 inverseProjView;
in mat4 inverseProj;

uniform sampler2D depthTex;
uniform sampler2D normalTex;
uniform sampler2D noiseTex;

uniform int kernelSize;
uniform vec2 pixelSize;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

uniform vec3 samples[32];

// tile noise texture over screen based on screen dimensions divided by noise size
//const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); // screen = 800x600

void main()
{
    vec2 noiseScale = vec2(1.0f) / pixelSize / vec2(4.0f);
    
    
    vec3 fragPos = vec3(( gl_FragCoord.x * pixelSize.x), (gl_FragCoord.y * pixelSize.y), 0.0);
    fragPos.z = texture(depthTex , fragPos.xy).r;
    
    vec3 normal = normalize(texture(normalTex, fragPos.xy).xyz * 2.0f - 1.0f);
    vec3 randomVec = texture(noiseTex, fragPos.xy * noiseScale).xyz * 2.0f - 1.0f;
    
    vec4 clip = inverseProjView * vec4(fragPos * 2.0f - 1.0f, 1.0f);
    
    fragPos = clip.xyz / clip.w;
    float fragDepth = (viewMatrix * vec4(fragPos, 1.0f)).z;
    
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    mat4 vpMatrix = projMatrix * viewMatrix;
    
    const float radius = 1.0f;
    float occlusion = 0.0f;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // From tangent to view-space
        sample = fragPos + sample * radius; 
        
        
        //turn sample to screen space
        vec4 screenSample = vec4(sample, 1.0f);
        screenSample = vpMatrix * screenSample; // from view to clip-space
        screenSample.xyz /= screenSample.w; // perspective divide
        screenSample.xyz = screenSample.xyz * 0.5f + 0.5f; // transform to range 0.0 - 1.0
        
        //new sample depth based on depth texture
        screenSample.z = texture(depthTex , screenSample.xy).r;
        screenSample = inverseProj * vec4(screenSample.xyz * 2.0f - 1.0f, 1.0f); //from clip-space to view
        float sampleDepth = screenSample.z / screenSample.w;
        sample.z = (viewMatrix * vec4(sample, 1.0f)).z;
        
        /*
        vec4 screenSample = vec4(sample, 1.0f);
        screenSample = vpMatrix * screenSample; // from view to clip-space
        screenSample.xyz /= screenSample.w; // perspective divide
        screenSample.xyz = screenSample.xyz * 0.5f + 0.5f; // transform to range 0.0 - 1.0
        
        // get sample depth
        screenSample.z = texture(depthTex, screenSample.xy).r; // Get depth value of kernel sample
        screenSample = inverseProj * vec4(screenSample.xyz * 2.0f - 1.0f, 1.0f); //from clip-space to view
        float sampleDepth = screenSample.z / screenSample.w;
        */
        
        //prevent far background objects interacting with close foreground objects
        float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(fragDepth - sampleDepth));
        occlusion += (sampleDepth >= sample.z ? 1.0f : 0.0f) * rangeCheck;
    }
    
    occlusion = 1.0f - (occlusion / float(kernelSize));
    FragColor = occlusion;
}