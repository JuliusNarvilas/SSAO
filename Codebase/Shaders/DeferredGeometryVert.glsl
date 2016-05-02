#version 330

in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out VERTEX {
    vec3 normal;
    vec2 texCoord;
} vs_out;

void main()
{
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
    
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vs_out.normal = normalMatrix * normal;
    
    vs_out.texCoord = texCoord;
}