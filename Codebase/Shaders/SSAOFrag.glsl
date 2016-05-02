#version 330 core

out float FragColor;

in VERTEX {
    mat4 inverseProjView;
} fs_in;

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
    vec2 noiseScale = vec2(1.0) / pixelSize / vec2(4.0);
    
    
    vec3 fragPos = vec3(( gl_FragCoord.x * pixelSize.x), (gl_FragCoord.y * pixelSize.y), 0.0);
    fragPos.z = texture(depthTex , fragPos.xy).r;
    
    vec3 normal = normalize(texture(normalTex, fragPos.xy).xyz * 2.0 - 1.0);
    vec3 randomVec = texture(noiseTex, fragPos.xy * noiseScale).xyz * 2.0 - 1.0;
    
    vec4 clip = fs_in.inverseProjView * vec4(fragPos * 2.0 - 1.0, 1.0);
    
    fragPos = clip.xyz / clip.w;
    
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        float radius = 1.0;
        // get sample position
        vec3 sample = TBN * samples[i]; // From tangent to view-space
        sample = fragPos + sample * radius; 
        
        //turn sample to screen space
        vec4 screenSample = vec4(sample, 1.0);
        screenSample = projMatrix * viewMatrix * screenSample; // from view to clip-space
        screenSample.xyz /= screenSample.w; // perspective divide
        screenSample.xyz = screenSample.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        //new sample depth based on depth texture
        screenSample.z = texture(depthTex , screenSample.xy).r;
        screenSample = fs_in.inverseProjView * vec4(screenSample.xyz * 2.0 - 1.0, 1.0); //from clip-space to view
        float fragDepth = screenSample.z / screenSample.w;
        
        //prevent far background objects interacting with close foreground objects
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - fragDepth));
        occlusion += (fragDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;
    }
    
    occlusion = 1.0 - (occlusion / float(kernelSize));
    FragColor = occlusion;
}