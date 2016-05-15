#version 330 core

out float FragColor;

uniform sampler2D ssaoTex;
uniform vec2 pixelSize;

void main()
{
    vec2 srcTexelSize = 1.0 / vec2(textureSize(ssaoTex, 0));
    vec2 texCoord = gl_FragCoord.xy * pixelSize;
    
    float result = 0.0f;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * srcTexelSize;
            result += texture(ssaoTex, texCoord + offset).r;
        }
    }
    FragColor = result * 0.0625f; //0.0625 = 1/16
}
