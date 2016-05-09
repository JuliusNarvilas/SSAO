#version 330 core

out float FragColor;

uniform sampler2D texIn;
uniform vec2 pixelSize;

void main()
{
    vec2 srcTexelSize = 1.0 / vec2(textureSize(texIn, 0));
    vec2 texCoord = gl_FragCoord.xy * pixelSize;
    
    vec3 result = vec3(0.0);
    int squareHalfEdge = 2;
    for (int x = -squareHalfEdge; x < squareHalfEdge; ++x) 
    {
        for (int y = -squareHalfEdge; y < squareHalfEdge; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * srcTexelSize;
            result += texture(texIn, texCoord + offset).rgb;
        }
    }
    FragColor = result / (squareHalfEdge * squareHalfEdge * 4.0);
}
