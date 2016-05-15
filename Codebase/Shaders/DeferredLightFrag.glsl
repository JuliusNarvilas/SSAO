#version 330 core

layout (location = 0) out vec3 out_emissColour;
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
const vec2 gridSamplingDisk[8] = vec2[]
(
vec2(-0.1242679f, -0.1288913f),
vec2(0.6947502f, 0.01457239f),
vec2(-0.6149264f, -0.2884163f),
vec2(0.5010659f, 0.5148036f),
vec2(0.3381125f, -0.4074082f),
vec2(-0.267173f, -0.7710676f),
vec2(-0.1626447f, 0.7281284f),
vec2(-0.746909f, 0.3824375f)
);

//generated poisson disk
const vec2 gridSamplingDisk2[16] = vec2[]
(
    vec2(-0.9009076, 0.420709),
    vec2(-0.3697696, -0.8962931),
    vec2(0.9322448, 0.2348164),
    vec2(-0.791388, -0.5095884),
    vec2(0.1102971, -0.9344642),
    vec2(0.5520206, 0.7520723),
    vec2(0.8444887, -0.2492936),
    vec2(-0.1042226, 0.8392674),
    vec2(-0.4930718, 0.5811127),
    vec2(0.4155082, -0.6237466),
    vec2(-0.719794, 0.01294611),
    vec2(0.519933, 0.164727),
    vec2(-0.06045581, -0.4244101),
    vec2(-0.08439049, 0.3641938),
    vec2(0.2785021, -0.1557052),
    vec2(-0.2758212, -0.04620392)
);

// For Poisson Disk PCF sampling
const vec2 gridSamplingDisk3[64] = vec2[]
(
    vec2(0.7418533f, -0.6667366f),
    vec2(0.4262091f, -0.9013284f),
    vec2(0.7375497f, 0.6691415f),
    vec2(0.1268544f, -0.9874692f),
    vec2(-0.9879128f, 0.1113683f),
    vec2(0.4216993f, 0.9002838f),
    vec2(-0.09640376f, 0.9843736f),
    vec2(-0.1658188f, -0.9695674f),
    vec2(0.9055501f, 0.3758393f),
    vec2(0.9705266f, -0.1143304f),
    vec2(0.9670017f, 0.1293385f),
    vec2(-0.3448896f, -0.9046497f),
    vec2(-0.9466816f, -0.2014508f),
    vec2(0.9015037f, -0.3306949f),
    vec2(0.5663417f, 0.7708698f),
    vec2(-0.8706837f, 0.3010679f),
    vec2(-0.5938849f, -0.6895654f),
    vec2(-0.5085648f, 0.7534177f),
    vec2(0.7713396f, -0.4713659f),
    vec2(0.2312844f, 0.8725659f),
    vec2(-0.7136765f, -0.4496614f),
    vec2(-0.8409063f, -0.03465778f),
    vec2(0.2001408f, -0.808381f),
    vec2(0.04146584f, 0.8313184f),
    vec2(-0.2483695f, 0.7942952f),
    vec2(-0.6707711f, 0.4912741f),
    vec2(0.4060591f, -0.7100726f),
    vec2(0.0005130528f, -0.8058334f),
    vec2(-0.7552931f, -0.2426507f),
    vec2(0.7599946f, 0.1809109f),
    vec2(0.5679897f, 0.5343465f),
    vec2(-0.3148003f, -0.7047654f),
    vec2(0.7682328f, -0.07273844f),
    vec2(0.573212f, -0.51544f),
    vec2(-0.6982038f, 0.1904326f),
    vec2(0.6813727f, -0.2424808f),
    vec2(-0.5119625f, -0.4827938f),
    vec2(-0.4241052f, 0.5581087f),
    vec2(0.6155105f, 0.3245716f),
    vec2(0.3492522f, 0.5924662f),
    vec2(0.149394f, 0.6650763f),
    vec2(-0.1020106f, 0.6724468f),
    vec2(0.2271994f, -0.6163502f),
    vec2(-0.6517572f, -0.07476326f),
    vec2(0.02703013f, -0.6010728f),
    vec2(0.5838975f, 0.1054943f),
    vec2(0.3928624f, -0.4417621f),
    vec2(-0.2171264f, -0.4768726f),
    vec2(-0.5082307f, 0.1079806f),
    vec2(0.3780522f, 0.3478679f),
    vec2(-0.3859636f, 0.3363545f),
    vec2(-0.42215f, -0.2024607f),
    vec2(0.1749884f, -0.4202175f),
    vec2(-0.0684978f, 0.4461993f),
    vec2(0.1507788f, 0.4204168f),
    vec2(0.3956799f, -0.1469177f),
    vec2(0.3514056f, 0.09865579f),
    vec2(-0.3042712f, -0.02195431f),
    vec2(0.1975043f, 0.2221317f),
    vec2(-0.1925334f, 0.1787288f),
    vec2(-0.08429877f, -0.2316298f),
    vec2(0.1558783f, -0.08460935f),
    vec2(0.003256182f, 0.138135f),
    vec2(-0.1041822f, -0.02521214f)
);

float ShadowCalculationTest(vec3 lightDir, float lightDist, float slope)
{   
    float shadow = 0.0;
    const float breakCondition = 0.001;
    const float offsetRadius = 0.005;
    
    vec3 dir1 = vec3(1.0, 1.0, 1.0);
    dir1 = normalize(dir1 - lightDir * dot(dir1, lightDir));
    vec3 dir2 = cross(lightDir, dir1) * offsetRadius;
    dir1 *= offsetRadius;
    
    //weighted bias based on slope
    float bias = 0.05 + 0.05 * tan(acos(slope));
    
    int testCount = 8;
    for(int i = 0; i < testCount; ++i)
    {
        float closestDepth = texture(depthMap, lightDir + dir1 * gridSamplingDisk3[i].x + dir2 * gridSamplingDisk3[i].y).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((lightDist - bias) > closestDepth)
        {
            shadow += 1.0;
        }
    }
    float test = shadow / float(testCount);
 
    shadow = 0.0;
    for(int i = 0; i < 64; ++i)
    {
        float closestDepth = texture(depthMap, lightDir + dir1 * gridSamplingDisk3[i].x + dir2 * gridSamplingDisk3[i].y).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((lightDist - bias) > closestDepth)
        {
            shadow += 1.0;
        }
    }
    shadow /= 64;
    
    if(test >= 0.99)
    {
        if(shadow >= 0.99)
        {
            return 0.0;
        }
        else
        {
            return 2.0f;
        }
    }
    return 1.0f;
}

float ShadowCalculation(vec3 lightDir, float lightDist, float slope)
{
    float shadow = 0.0;
    const float breakCondition = 0.33;
    const float offsetRadius = 0.005;
    const int samples = 64;
    
    vec3 dir1 = vec3(1.0, 1.0, 1.0);
    dir1 = normalize(dir1 + lightDir * dot(dir1, -lightDir));
    vec3 dir2 = cross(-lightDir, dir1) * offsetRadius;
    dir1 *= offsetRadius;
    
    //weighted bias based on slope
    float bias = 0.05 + 0.03 * tan(acos(slope));
    
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, -lightDir + dir1 * gridSamplingDisk3[i].x + dir2 * gridSamplingDisk3[i].y).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((lightDist - bias) > closestDepth)
        {
            shadow += 1.0;
        }
    }
    return shadow / float(samples);
}

float ShadowCalculationFast(vec3 lightDir, float lightDist, float slope)
{
    float shadow = 0.0f;
    const float offsetRadius = 0.005f;
    const int samples = 16;
    const int earlyTestSamples = 8;
    
    vec3 dir1 = vec3(1.0f, 1.0f, 1.0f);
    dir1 = normalize(dir1 - lightDir * dot(dir1, lightDir));
    vec3 dir2 = cross(lightDir, dir1) * offsetRadius;
    dir1 *= offsetRadius;
    
    //weighted bias based on slope
    float bias = 0.05f + 0.03f * tan(acos(slope));
    
    for(int i = 0; i < earlyTestSamples; ++i)
    {
        float closestDepth = texture(depthMap, lightDir + dir1 * gridSamplingDisk2[i].x + dir2 * gridSamplingDisk2[i].y).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((lightDist - bias) > closestDepth)
        {
            shadow += 1.0f;
        }
    }
    if(shadow <= 0.001f)
    {
        return 0.0f;
    }
    if(shadow >= 7.99f)
    {
        return 1.0f;
    }
    
    for(int i = earlyTestSamples; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, lightDir + dir1 * gridSamplingDisk2[i].x + dir2 * gridSamplingDisk2[i].y).r;
        closestDepth *= lightRadius;   // Undo mapping [0;1]
        if((lightDist - bias) > closestDepth)
        {
            shadow += 1.0f;
        }
    }
    return shadow / float(samples);
}

float ShadowCalculationSimple(vec3 lightDir, float lightDist, float slope)
{
    float closestDepth = texture(depthMap, lightDir).r;
    closestDepth *= lightRadius;   // Undo mapping [0;1]
    float bias = 0.05 +  0.05 * tan(acos(slope));
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


    vec3 dirToLight = lightPos - pos;
    float dist = length(dirToLight);
    dirToLight = normalize(dirToLight);

    float slopeFactor = dot(dirToLight, normal);
    float lambert = max(slopeFactor, 0.0);
    
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
    vec3 halfDir = normalize(dirToLight + viewDir);
    float specAngle = max(dot(halfDir, normal), 0.0);
    float specFactor = pow(specAngle, 33.0);
    
    float shadow = ShadowCalculationFast(-dirToLight, dist, slopeFactor);
    float lightingFactor = (1.0 - shadow) * atten;

    out_emissColour = lightingFactor * lightColour * lambert;
	out_specColour = lightingFactor * lightColour * specFactor;
    
    /*
    shadow = ShadowCalculationTest(-dirToLight, dist, slopeFactor);
    
    if(shadow <= 0.1)
    {
        out_emissColour = vec3(1.0, 1.0, 1.0);
    }
    else if(shadow <= 1.1)
    {
        out_emissColour = vec3(1.0, 0.0, 0.0);
    }
    else if(shadow <= 2.1)
    {
        out_emissColour = vec3(0.0, 1.0, 0.0);
    }
    else if(shadow <= 3.1)
    {
        out_emissColour = vec3(0.0, 0.0, 1.0);
    }
    else
    {
        out_emissColour = vec3(1.0, 1.0, 0.0);
    }
    out_specColour = vec3(0.0, 0.0, 0.0);
    */
}