#version 330 core

layout (location = 0) out vec2 out_normal;
layout (location = 1) out vec4 out_colour;

in VERTEX {
    vec3 normal;
    vec2 texCoord;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

vec2 encode (vec3 n)
{
    float f = sqrt(8.0f * n.z + 8.0f);
    return n.xy / f + 0.5f;
}
vec3 decode (vec2 enc)
{
    vec2 fenc = enc * 4.0f - 2.0f;
    float f = dot(fenc,fenc);
    float g = sqrt(1.0f - f / 4.0f);
    vec3 n;
    n.xy = fenc * g;
    n.z = 1.0 - f / 2.0f;
    return n;
}

void main()
{
    //mat3  TBN = mat3(IN.tangent , IN.binormal , IN.normal );
    //vec3  normal = normalize(TBN * (texture(bumpTex , IN.texCoord ).rgb) * 2.0f - 1.0f);
    
    // Also store the per-fragment normals into the gbuffer
    out_normal = encode(normalize(fs_in.normal));
    
    // And the diffuse per-fragment color
    out_colour.rgb = texture(texture_diffuse1, fs_in.texCoord).rgb;
    
    // Store specular intensity in gAlbedoSpec's alpha component
    out_colour.a = texture(texture_specular1, fs_in.texCoord).r;
}