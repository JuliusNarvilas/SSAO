#version 330 core

in vec3 position;

uniform mat4 projMatrix;
uniform mat4 orthoProjMatrix;
uniform mat4 viewMatrix;

out mat4 inverseProj;
out mat4 inverseProjView;
out mat4 vpMatrix;

void main()
{
    gl_Position = orthoProjMatrix * vec4(position, 1.0);
    inverseProjView = inverse(projMatrix * viewMatrix);
    inverseProj = inverse(projMatrix);
    vpMatrix = projMatrix * viewMatrix;
}