#version 330 core

layout (location = 0) out vec3 out_emissColour;
layout (location = 1) out vec3 out_specColour;

uniform sampler2D   depthTex;
uniform sampler2D   normTex;
uniform sampler2D   noiseTex;
uniform samplerCube depthMap;

uniform vec2        pixelSize;
uniform vec3        cameraPos;

uniform float       lightRadius;
uniform vec3        lightPos;
uniform vec3        lightColour;

in VERTEX {
    mat4 inverseProjView;
} fs_in;



// array of offset direction for sampling
const vec3 gridSamplingDisk[26] = vec3[]
(
//6 (+6 faces)
    normalize(vec3( 1.0,  0.0,  0.0)), normalize(vec3( 0.0,  1.0,  0.0)), normalize(vec3( 0.0,  0.0,  1.0)),
    normalize(vec3(-1.0,  0.0,  0.0)), normalize(vec3( 0.0, -1.0,  0.0)), normalize(vec3( 0.0,  0.0, -1.0)),
//14 (+8 corners)
    normalize(vec3( 1.0,  1.0,  1.0)), normalize(vec3(-1.0, -1.0, -1.0)),
    normalize(vec3( 1.0, -1.0, -1.0)), normalize(vec3(-1.0,  1.0,  1.0)),
    normalize(vec3(-1.0, -1.0,  1.0)), normalize(vec3( 1.0,  1.0, -1.0)),
    normalize(vec3(-1.0,  1.0, -1.0)), normalize(vec3( 1.0, -1.0,  1.0)),
//18 (+4 xz plane corners)
    normalize(vec3( 1.0,  0.0,  1.0)), normalize(vec3(-1.0,  0.0, -1.0)),
    normalize(vec3( 1.0,  0.0, -1.0)), normalize(vec3(-1.0,  0.0,  1.0)),
//22 (+4 yx plane corners)
    normalize(vec3( 1.0,  1.0,  0.0)), normalize(vec3(-1.0, -1.0,  0.0)),
    normalize(vec3( 1.0, -1.0,  0.0)), normalize(vec3(-1.0,  1.0,  0.0)),
//26 (+4 yz plane corners)
    normalize(vec3( 0.0,  1.0,  1.0)), normalize(vec3( 0.0, -1.0, -1.0)),
    normalize(vec3( 0.0,  1.0, -1.0)), normalize(vec3( 0.0, -1.0,  1.0))
);

// array of offset direction for sampling
const vec2 gridSamplingDisk2[8] = vec2[]
(
    vec2( 1.0,  0.0), vec2( 0.0,  1.0), vec2(-1.0,  0.0), vec2( 0.0, -1.0),
    normalize(vec2( 1.0,  1.0)), normalize(vec2(-1.0, -1.0)),
    normalize(vec2(-1.0,  1.0)), normalize(vec2( 1.0, -1.0))
);

//float bias = 0.05; //* tan(acos(slope)) * max(viewDistance / lightRadius, 0.25);
const float bias = 0.0;
const float offsetRadius = 0.0;

float ShadowCalculationTest(vec3 fragPos, float slope)
{   
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    
    float shadow = 0.0;
    const float breakCondition = 0.33;
    for(int i = 0; i < 6; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * offsetRadius).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((currentDepth - bias) > closestDepth)
        {
            shadow += 1.0;
        }
    }
    if(shadow / 6.0 <= breakCondition) return 0.0;
    
    for(int i = 6; i < 14; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * offsetRadius).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((currentDepth - bias) > closestDepth)
        {
            shadow += 1.0;
        }
    }
    if(shadow / 14.0 <= breakCondition) return 1.0;
    
    for(int i = 14; i < 26; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * offsetRadius).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((currentDepth - bias) > closestDepth)
        {
            shadow += 1.0;
        }
    }
    if(shadow / 26.0 <= breakCondition) return 2.0;
    
    return 3.0;
}

float ShadowCalculation(vec3 lightDir, float lightDist, float slope, vec3 fragPos, vec3 normal)
{
    float shadow = 0.0;
    const float breakCondition = 0.33;
    const float offsetRadius = 0.003;
    const int samples = 8;
    
    vec2 noiseScale = vec2(1.0) / pixelSize / vec2(4.0);
    vec2 randomVec = normalize(texture(noiseTex, fragPos.xy * noiseScale).xy * 2.0 - 1.0) * 0.002;
    vec3 dir1 = vec3(1.0, 1.0, 1.0);
    dir1 = normalize(dir1 - normal * dot(dir1, normal)) * offsetRadius;
    vec3 dir2 = cross(normal, dir1) * offsetRadius;
    
    //weighted bias based on slope
    float bias = 0.05 * tan(acos(slope));
    
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, -lightDir + dir1 * gridSamplingDisk2[i].x + dir2 * gridSamplingDisk2[i].y).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((lightDist - bias) > closestDepth)
        {
            shadow += 1.0;
        }
    }
    dir1 *= 0.5;
    dir2 *= 0.5;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, -lightDir + dir1 * gridSamplingDisk2[i].x + dir2 * gridSamplingDisk2[i].y).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((lightDist - bias) > closestDepth)
        {
            shadow += 1.0;
        }
    }
    return shadow / float(samples * 2);
}

float SimpleShadowCalculation(vec3 lightDir, float lightDist, float slope)
{
    float closestDepth = texture(depthMap, -lightDir).r;
    closestDepth *= lightRadius;   // Undo mapping [0;1]
    float bias = 0.05 * tan(acos(slope));
    if((lightDist - bias) > closestDepth)
    {
        return 1.0;
    }
    return 0.0;
}



void  main(void)    {
    vec3 pos = vec3(( gl_FragCoord.x * pixelSize.x), (gl_FragCoord.y * pixelSize.y), 0.0);
    pos.z = texture(depthTex , pos.xy).r;

    vec3 normal = normalize(texture(normTex, pos.xy).xyz * 2.0 - 1.0);

    vec4 clip = fs_in.inverseProjView * vec4(pos * 2.0 - 1.0, 1.0);
    pos = clip.xyz / clip.w;


    vec3 lightDir = lightPos - pos;
    float dist = length(lightDir);
    lightDir = normalize(lightDir);

    float lambert = max(dot(lightDir,normal), 0.0);
    
    if(lambert < 0.001)
    {
        discard;
    }
    
    float atten = 1.0 - min(dist / lightRadius, 1.0);
    if(atten < 0.001)
    {
        discard;
    }
    vec3 viewDir = normalize(cameraPos - pos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normal), 0.0);
    float specFactor = pow(specAngle, 33.0);
    
    float shadow = ShadowCalculation(lightDir, dist, dot(lightDir, normal), pos, normal);
    float lightingFactor = (1.0 - shadow) * atten;

    out_emissColour = lightingFactor * lightColour * lambert;
	out_specColour = lightingFactor * lightColour * specFactor;
    
    /*
    float shadow = SimpleShadowCalculation(pos, dot(lightDir, normal));
    
    if(shadow <= 0.1)
    {
        out_emissColour = vec3(1.0, 1.0, 1.0);
    }
    else if(shadow <= 1.1)
    {
        out_emissColour = vec3(1.0, 0.75, 0.75);
    }
    else if(shadow <= 2.1)
    {
        out_emissColour = vec3(0.5, 75.0, 0.5);
    }
    else if(shadow <= 3.1)
    {
        out_emissColour = vec3(0.1, 0.1, 0.1);
    }
    */
}