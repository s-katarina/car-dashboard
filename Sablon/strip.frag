#version 330 core

out vec4 outCol;

uniform float uR;
uniform float uB;

void main()
{
	outCol = vec4(uR, 0.0, uB, 1.0);
}
