#version 330 core

in vec3 position;

uniform mat4 projMatrix;
uniform mat4 orthoProjMatrix;
uniform mat4 viewMatrix;

out VERTEX {
     mat4 inverseProjView;
} vs_out;


void main()
{
    gl_Position = orthoProjMatrix * vec4(position, 1.0);
    vs_out.inverseProjView = inverse(projMatrix * viewMatrix);
}