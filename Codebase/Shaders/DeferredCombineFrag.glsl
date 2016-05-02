#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D emissiveTex;
uniform sampler2D specularTex;
uniform sampler2D ssaoTex;

in VERTEX {
    vec2 texCoord;
} IN;

out  vec3  FragColor;

void  main(void)    {
    vec3 diffuse = texture(diffuseTex, IN.texCoord).xyz;
    vec3 light = texture(emissiveTex, IN.texCoord).xyz;
    vec3 specular = texture(specularTex, IN.texCoord).xyz;

    FragColor = diffuse * (0.3 * texture(ssaoTex, IN.texCoord).r);    // ambient
    FragColor += diffuse * light; // lambert
    FragColor += specular;        // Specular
}