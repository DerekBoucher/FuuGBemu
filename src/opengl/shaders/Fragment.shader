#shadertype fragment

#version 330 core

in vec4 outputColor;

out vec4 FragColor;

void main()
{
    FragColor = outputColor;
}
