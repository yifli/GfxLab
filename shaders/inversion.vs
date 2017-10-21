#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texcoord;

out vec2 vs_texcoord;
void main()
{
	gl_Position = vec4(pos, 0.0, 1.0);
	vs_texcoord = texcoord;
}