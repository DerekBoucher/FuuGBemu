#shadertype vertex

#version 330 core

layout (location = 0) in vec2 inputPosition;
layout (location = 1) in vec3 inputColor;
out vec4 outputColor;

void main() 
{
    outputColor = vec4(inputColor, 1.0);
    gl_Position = vec4(inputPosition.x, inputPosition.y, 0.0, 1.0);
}
