#version 330 core
const vec3 light_color = vec3(1.0, 1.0, 1.0);

in vec3 frag_pos;
in vec3 frag_normal;
in vec3 obj_color;

out vec4 frag_color;

uniform vec3 light_pos;

void main()
{
	float ambient_strength = 0.1;
	vec3 ambient = ambient_strength * light_color;
	vec3 norm = normalize(frag_normal);
	vec3 light_dir = normalize(light_pos - frag_pos);
	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = diff * light_color;
	frag_color = vec4((ambient+diffuse) * obj_color, 1.0);
}
