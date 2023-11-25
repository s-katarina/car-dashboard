#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inCol;
out vec3 chCol;

uniform mat4 trans;
uniform float uPosX;


void main()
{
	gl_Position = trans * vec4(inPos.x + uPosX, inPos.y, 0.0, 1.0);
	chCol = inCol;
}