#version 330 core

in vec3 position;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out VERTEX {
    mat4 inverseProjView;
} vs_out;


void main()
{
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
    vs_out.inverseProjView = inverse(projMatrix * viewMatrix);
}