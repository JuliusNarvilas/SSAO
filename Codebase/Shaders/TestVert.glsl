#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} vs_out;


void main()
{
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
    vs_out.FragPos = vec3(modelMatrix * vec4(position, 1.0));
    vs_out.Normal = transpose(inverse(mat3(modelMatrix))) * normal;
    vs_out.TexCoord = texCoord;
}