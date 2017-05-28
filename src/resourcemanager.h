#pragma once

#include "common.h"


class ResourceManager {
public:
	ResourceManager();
	void    LoadMesh(const std::string& file, MeshPtr& pMesh);
	GLuint  LoadTexture(const std::string& type, const std::string& path);
	GLuint  CreateProgram(std::vector<std::string>& shader_files);
private:
	GLuint  CreateShader(const std::string& file);

	using VAOPtr     = std::unique_ptr<GLuint, std::function<void(GLuint*)>>;
	using TexObjPtr  = std::unique_ptr<GLuint, std::function<void(GLuint*)>>;
	using ShaderPtr  = std::unique_ptr<GLuint, std::function<void(GLuint*)>>;
	using ProgramPtr = std::unique_ptr<GLuint, std::function<void(GLuint*)>>;

	std::unordered_map<std::string, VAOPtr>     _vertex_array_objs;
	std::unordered_map<std::string, TexObjPtr>  _texture_objs;
	std::unordered_map<std::string, ShaderPtr>  _shaders;
	std::unordered_map<std::string, ProgramPtr> _programs;

	std::function<void(GLuint*)>               _vao_deleter;
	std::function<void(GLuint*)>               _texobj_deleter;
	std::function<void(GLuint*)>               _shader_deleter;
	std::function<void(GLuint*)>               _program_deleter;
};
