#version 330 core
layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec3 color;

out vec3 frag_pos;
out vec3 frag_normal;
out vec3 obj_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	obj_color = color;
	frag_pos = vec3(model * vec4(position, 1.0));
	frag_normal = normal;
}