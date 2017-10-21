#version 330 core

in vec2 vs_texcoord;

uniform sampler2D screen_tex;

out vec4 frag_color;

void main()
{
	frag_color = 1.0 - texture(screen_tex, vs_texcoord);
}