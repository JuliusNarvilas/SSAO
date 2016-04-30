#version 330 core

layout (location = 0) out vec3 out_diffColour;
layout (location = 1) out vec3 out_specColour;

uniform  sampler2D     depthTex;
uniform  sampler2D     normTex;

uniform  vec2           pixelSize;
uniform  vec3           cameraPos;

uniform  float          lightRadius;
uniform  vec3           lightPos;
uniform  vec4           lightColour;

in VERTEX {
    mat4 inverseProjView;
} fs_in;

void  main(void)    {
    vec3 pos = vec3(( gl_FragCoord.x * pixelSize.x), (gl_FragCoord.y * pixelSize.y), 0.0);
    pos.z = texture(depthTex , pos.xy).r;

    vec3 normal = normalize(texture(normTex, pos.xy).xyz * 2.0 - 1.0);

    vec4 clip = fs_in.inverseProjView * vec4(pos * 2.0 - 1.0, 1.0);
    pos = clip.xyz / clip.w;

    float dist = length(lightPos - pos);
    float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);

    //if(atten == 0.0) {
    //    discard;
    //}

    vec3  incident = normalize(lightPos - pos);
    vec3  viewDir = normalize(cameraPos - pos);
    vec3  halfDir = normalize(incident + viewDir );

    float  lambert = clamp(dot(incident, normal), 0.0, 1.0);
    float  rFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
    float  sFactor = pow(rFactor, 33.0);

    out_diffColour = lightColour.xyz * lambert * atten;
    out_specColour = lightColour.xyz * sFactor * atten * 0.33;
}