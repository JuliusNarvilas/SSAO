#version 330

in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out VERTEX {
    //vec3 position;
    vec3 normal;
    vec2 texCoord;
} vs_out;

void main()
{
    vec4 worldPos = modelMatrix * vec4(position, 1.0f);
    gl_Position = projMatrix * viewMatrix * worldPos;
    //vs_out.position = worldPos.xyz;
    
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vs_out.normal = normalMatrix * normal;
    
    vs_out.texCoord = texCoord;
}