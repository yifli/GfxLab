#pragma once

#include "common.h"
#include "material.h"

struct BoundingBox {
    glm::vec3	min;
    glm::vec3	max;
    glm::vec3	center;
};

class Geometry {
    friend class ResourceManager;
public:
    Geometry()
        : _texture(0), _vao(0), _vbo(0), _ibo(0), _transparency(1.0f)
    {}

    virtual void       Render() = 0;
    void               ApplyTransformation(const glm::mat4& trans) { _transformation *= trans; }
    void               SetTexture(GLenum type, GLuint texture)     { _textureType = type; _texture = texture; }
    bool               UsesTexture() const                         { return _texture != 0; }
    void               SetMaterial(const Material& mat)            { _material = mat; }
    void               SetTransparency(float t)                    { _transparency = t; }
    void               SetShaderProgram(GLuint program)            { _program = program; }
    const glm::mat4&   GetTransformation() const                   { return _transformation; }
    const BoundingBox& GetBoundingBox()    const                   { return _bbox; }
    GLuint             GetShaderProgram()  const                   { return _program; }
    const std::string& GetName() const                             { return _id; }
    void               SetName(const std::string name)             { _id = name; }
 
protected:
    BoundingBox _bbox;
    glm::mat4   _transformation;
    Material    _material;
    float       _transparency;
    GLenum      _textureType;
    GLuint      _texture;
    GLuint      _vao;
    GLuint      _vbo;
    GLuint      _ibo;
    GLuint      _program;
    std::string _id;
};