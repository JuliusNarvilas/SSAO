#version 330 core

uniform  sampler2D  diffuseTex;
uniform  sampler2D  emissiveTex;
uniform  sampler2D  specularTex;

in VERTEX {
    vec2 texCoord;
} IN;

out  vec4  FragColor;

void  main(void)    {
    vec3 diffuse = texture(diffuseTex, IN.texCoord).xyz;
    vec3 light = texture(emissiveTex, IN.texCoord).xyz;
    vec3 specular = texture(specularTex, IN.texCoord).xyz;

    FragColor.xyz = diffuse * 0.2;    // ambient
    FragColor.xyz += diffuse * light; // lambert
    FragColor.xyz += specular;        // Specular
    FragColor.a = 1.0;
}