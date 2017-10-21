#version 330 core
const vec3 light_color = vec3(1.0, 1.0, 1.0);

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_texcoord;

uniform vec3 light_pos;
uniform sampler2D diffuse_map;

out vec4 frag_color;

void main()
{
	float ambient_strength = 0.1;
	vec3 ambient = ambient_strength * light_color;
	vec3 light_dir = light_pos - frag_pos;
	float diffuse_strength = max(0.0, normalize(dot(light_dir, frag_normal)));
	vec3 diffuse = diffuse_strength * light_color;
	frag_color = vec4((ambient+diffuse) * vec3(texture(diffuse_map, frag_texcoord)), 1.0);
}