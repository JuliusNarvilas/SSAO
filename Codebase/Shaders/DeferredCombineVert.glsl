#version 330

uniform  mat4  projMatrix;

in vec3 position;
in vec2 texCoord;

out VERTEX {
    vec2 texCoord;
} vs_out;

void main(void) {
    gl_Position = projMatrix * vec4(position, 1.0);
    vs_out.texCoord = texCoord;
}
