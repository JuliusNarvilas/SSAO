#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

uniform sampler2D diffuseTex;
uniform samplerCube depthMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;

out vec4 FragColor;


// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), 
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

float ShadowCalculation(vec3 fragPos, float slope)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // Get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // Test for shadows with PCF
    float shadow = 0.0;
    
    float viewDistance = length(viewPos - fragPos);
    
    //weighted bias based on slope
    float bias = 0.15 * tan(acos(slope)) * max(viewDistance / far_plane, 0.25);
    
    int samples = 20;
    int earlyBreak = 5;
    int loopSize = samples / earlyBreak;
    
    float diskRadius = 0.007 + viewDistance / 700.0;
    //angle factor is used to remove shadows from well light corners
    float oneMinusAngleToLightFactor = 1.0 - smoothstep(0.9, 1.0, slope);
    
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius * oneMinusAngleToLightFactor).r;
        closestDepth *= far_plane;   // Undo mapping [0;1]
        if((currentDepth - bias) > closestDepth)
        {
            shadow += 1.0;
        }
    }
    shadow /= float(samples);
        
    // Display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    // return shadow;
    return shadow;
}

void main()
{

    vec3 color = texture(diffuseTex, fs_in.TexCoord).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // Ambient
    vec3 ambient = 0.3 * color;
    // Diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float lambertian = max(dot(lightDir, normal), 0.0);
    
    //this assigns a gradient over 360 degrees and is vary choppy
    vec3 diffuse = lightColor; //*lambertian;
    float lightIntensity = 1.0 - smoothstep(0.0, far_plane, length(lightPos - fs_in.FragPos));
    
    // Specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // Calculate shadow
    
    float shadow = ShadowCalculation(fs_in.FragPos, dot(lightDir, normal));               
    
    vec3 lighting = (ambient + lightIntensity * (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0f);
}