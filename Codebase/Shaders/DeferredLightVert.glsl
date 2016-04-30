#version 330

in vec3 position;

uniform  mat4  modelMatrix;
uniform  mat4  viewMatrix;
uniform  mat4  projMatrix;
uniform  mat4  textureMatrix;

out VERTEX {
     mat4 inverseProjView;
} vs_out;

void main()
{
    gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(position, 1.0);
    
    vs_out.inverseProjView = inverse(projMatrix * viewMatrix);
}