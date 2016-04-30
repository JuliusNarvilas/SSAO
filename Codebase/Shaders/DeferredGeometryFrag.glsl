#version 330 core

//layout (location = 0) out vec3 out_position;
layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec4 out_colour;

in VERTEX {
    //vec3 position;
    vec3 normal;
    vec2 texCoord;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    //mat3  TBN = mat3(IN.tangent , IN.binormal , IN.normal );
    //vec3  normal = normalize(TBN * (texture(bumpTex , IN.texCoord ).rgb) * 2.0 - 1.0);
    
    // Store the fragment position vector in the first gbuffer texture
    //out_position = fs_in.position;
    
    // Also store the per-fragment normals into the gbuffer
    out_normal = normalize(fs_in.normal);// * 0.5 + 0.5;
    
    // And the diffuse per-fragment color
    out_colour.rgb = texture(texture_diffuse1, fs_in.texCoord).rgb;
    
    // Store specular intensity in gAlbedoSpec's alpha component
    out_colour.a = texture(texture_specular1, fs_in.texCoord).r;
}