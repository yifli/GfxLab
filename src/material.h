#ifndef MATERIAL_H_
#define MATERIAL_H

#include "common.h"

struct Material {
public:
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float     shininess;

	static const Material EMERALD;
	static const Material JADE;
	static const Material PEARL;
	static const Material RUBY;
	static const Material BRASS;
	static const Material BRONZE;
	static const Material SILVER;
	static const Material GOLD;
};

#endif

