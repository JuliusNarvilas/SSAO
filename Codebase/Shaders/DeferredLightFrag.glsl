#version 330 core

layout (location = 0) out vec3 out_diffColour;
layout (location = 1) out vec3 out_specColour;

uniform sampler2D   depthTex;
uniform sampler2D   normTex;
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

//float bias = 0.05; //* tan(acos(slope)) * max(viewDistance / lightRadius, 0.25);
const float bias = 0.05;
const float offsetRadius = 0.02;

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

float ShadowCalculation(vec3 fragPos, float slope)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // Get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // Test for shadows with PCF
    float shadow = 0.0;
    const float breakCondition = 0.33;
    
    float viewDistance = length(cameraPos - fragPos);
    
    //weighted bias based on slope
    float bias = 0.05; //* tan(acos(slope)) * max(viewDistance / lightRadius, 0.25);
    
    const int samples = 26;
    
    float offsetRadius = 0.05;// + viewDistance / 700.0;
    //angle factor is used to remove shadows from well light corners
    //float oneMinusAngleToLightFactor = 1.0 - smoothstep(0.9, 1.0, slope);
    
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * offsetRadius).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((currentDepth - bias) > closestDepth)
        {
            shadow += 1.0;
        }
    }
    return shadow / float(samples);
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
    float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);

    //if(atten == 0.0) {
    //    discard;
    //}

    float shadow = ShadowCalculation(pos, dot(lightDir, normal));

    vec3  incident = normalize(lightPos - pos);
    vec3  viewDir = normalize(cameraPos - pos);
    vec3  halfDir = normalize(incident + viewDir );

    float  lambert = clamp(dot(incident, normal), 0.0, 1.0);
    float  rFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
    float  sFactor = pow(rFactor, 33.0);

    out_diffColour = (1.0 - shadow) * lightColour * lambert * atten;
    out_specColour = lightColour * sFactor * atten * 0.33;
    
    shadow = 4.0;
    
    if(shadow <= 0.1)
    {
        out_diffColour = vec3(1.0, 1.0, 1.0);
    }
    else if(shadow <= 1.1)
    {
        out_diffColour = vec3(1.0, 0.75, 0.75);
    }
    else if(shadow <= 2.1)
    {
        out_diffColour = vec3(0.5, 75.0, 0.5);
    }
    else if(shadow <= 3.1)
    {
        out_diffColour = vec3(0.1, 0.1, 0.1);
    }
}