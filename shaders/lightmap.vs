#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec2 texcoord;

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0);
	frag_pos = vec3(view * model * vec4(position, 1.0));
	frag_normal = mat3(transpose(inverse(view*model))) * normal;
	frag_texcoord = texcoord;
}