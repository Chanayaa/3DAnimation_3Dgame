#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform int objectType;   // 0 = sphere, 1 = heart

void main()
{
    if(objectType == 0)
        FragColor = texture(texture1, TexCoord);
    else
        FragColor = texture(texture2, TexCoord);
}
