#version 330 core


out vec4 outCol;

uniform float uGearR;
uniform float uGearG;
uniform float uGearB;

void main()
{
	outCol = vec4(uGearR, uGearG, uGearB, 0.8);
}