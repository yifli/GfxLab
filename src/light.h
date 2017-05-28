#pragma once
#include "common.h"


class Light {
public:
	enum Type {
		DIRECTION,
		POINT,
		SPOT
	};

	Light()
		:_ambient(.2f, .2f, .2f),
		_diffuse(.5f, .5f, .5f),
		_specular(1.0f, 1.0f, 1.0f),
		_constant(1.0f),
		_linear(0.09f),
		_quadratic(0.032f),
		_type(POINT)
	{}

	void             SetType(const Type& type)               { _type = type; }
	void             SetPosition(const glm::vec3& pos)       { _position = pos; }
	void             SetDirection(const glm::vec3& dir)      { _direction = dir; }
	void             SetAmbient(const glm::vec3& ambient)    { _ambient = ambient; }
	void             SetDiffuse(const glm::vec3& diffuse)    { _diffuse = diffuse; }
	void             SetSpecular(const glm::vec3& specular)  { _specular = specular; }
	void             SetConstantAtten(float factor)          { _constant = factor; }
	void             SetLinearAtten(float factor)            { _linear = factor; }
	void             SetQuadraticAtten(float factor)         { _quadratic = factor; }
	Light::Type      GetType()                         const { return _type; }
	const glm::vec3& GetPosition()                     const { return _position; }
	const glm::vec3& GetDirection()                    const { return _direction; }
	const glm::vec3& GetAmbient()                      const { return _ambient; }
	const            glm::vec3& GetDiffuse()           const { return _diffuse; }
	const glm::vec3& GetSpecular()                     const { return _specular; }
	float            GetConstantAtten()                const { return _constant; }
	float            GetLinearAtten()                  const { return _linear; }
	float            GetQuadraticAtten()               const { return _quadratic; }



private:
	Type      _type;
	glm::vec3 _color;
	glm::vec3 _position;
	glm::vec3 _direction;
	glm::vec3 _ambient;
	glm::vec3 _diffuse;
	glm::vec3 _specular;
	float     _constant;
	float     _linear;
	float     _quadratic;
};