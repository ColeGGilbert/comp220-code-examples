#version 330 core

out vec3 color;
in vec2 randCol;

float rand(vec2 co)
{
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
	color = vec3(rand(randCol),rand(randCol),rand(randCol));	
}