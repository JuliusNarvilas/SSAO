#version 330 core

in vec3 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // Write this as modified depth
    //gl_FragDepth = gl_FragCoord.z;
    gl_FragDepth = lightDistance;
}