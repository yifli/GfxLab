#include "resourcemanager.h"
#include "mesh.h"

#include <SOIL.h>

#include <cstdlib>

ResourceManager::ResourceManager()
{
	_vao_deleter     = [](GLuint* id) { glDeleteVertexArrays(1, id); };
	_texobj_deleter  = [](GLuint* id) { glDeleteTextures(1, id); };
	_program_deleter = [](GLuint* id) { glDeleteProgram(*id); };
	_shader_deleter  = [](GLuint* id) { glDeleteShader(*id); };
}


void ResourceManager::LoadMesh(const std::string& file, MeshPtr& pMesh)
{
	auto mesh = pMesh->GetMeshObj();
	mesh.request_vertex_normals();
	mesh.request_vertex_texcoords2D();
	mesh.request_vertex_colors();

	OpenMesh::IO::Options opt;
	opt += OpenMesh::IO::Options::VertexNormal;
	opt += OpenMesh::IO::Options::VertexTexCoord;
	opt += OpenMesh::IO::Options::VertexColor;
	assert(OpenMesh::IO::read_mesh(mesh, file, opt));

	if (!opt.check(OpenMesh::IO::Options::VertexNormal)) {
		mesh.request_face_normals();
		mesh.update_normals();
		mesh.release_face_normals();
	}

	if (!opt.check(OpenMesh::IO::Options::VertexTexCoord))
		mesh.release_vertex_texcoords2D();

	if (!opt.check(OpenMesh::IO::Options::VertexColor))
		mesh.release_vertex_colors();


	if (_vertex_array_objs.find(file) == _vertex_array_objs.end()) {
		pMesh->SetupVAO();
		GLuint vao = pMesh->_vao;
		_vertex_array_objs[file] = std::unique_ptr<GLuint, decltype(_vao_deleter)>(new GLuint(vao), _vao_deleter);
	}
	else 
		pMesh->_vao = *(_vertex_array_objs[file]);
	
	pMesh->ComputeBoundingBox();
	pMesh->SetInitialTransformation();
}

GLuint ResourceManager::LoadTexture(const std::string& type, const std::string& path)
{
	if (_texture_objs.find(path) == _texture_objs.end()) {
		GLuint texobj;
		glGenTextures(1, &texobj);
		_texture_objs[path] = std::unique_ptr<GLuint, decltype(_texobj_deleter)>(new GLuint(texobj), _texobj_deleter);

		unsigned char* image;
		int width, height;
		if (type == "2D") {
			image = SOIL_load_image(path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
			if (!image) {
				std::cout << "Failed to load texture " << path << std::endl;
				assert(0);
			}

			glBindTexture(GL_TEXTURE_2D, texobj);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(GL_TEXTURE_2D);
			SOIL_free_image_data(image);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else if (type == "CubeMap") {
			glBindTexture(GL_TEXTURE_CUBE_MAP, texobj);
			std::vector<std::string> files = { "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "back.jpg", "front.jpg" };
			int i = 0;

			for (auto f : files) {
				std::string full_path = path + "/" + f;
				image = SOIL_load_image(full_path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
				if (!image) {
					std::cout << "Failed to load texture " << full_path << std::endl;
					assert(0);
				}
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i++, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
	}

	return *(_texture_objs[path]);
}


GLuint ResourceManager::CreateProgram(std::vector<std::string>& shader_files)
{
	std::sort(shader_files.begin(), shader_files.end());
	std::string str;
	for (auto& s : shader_files)
		str += s;

	if (_programs.find(str) == _programs.end()) {
		GLuint program = glCreateProgram();

		for (auto& s : shader_files)
			glAttachShader(program, CreateShader(s));

		glLinkProgram(program);
		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[512];
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		_programs[str] = std::unique_ptr<GLuint, decltype(_program_deleter)>(new GLuint(program), _program_deleter);
	}

	return *(_programs[str]);
}

GLuint ResourceManager::CreateShader(const std::string& file)
{
	if (_shaders.find(file) == _shaders.end()) {
		std::string suffix = file.substr(file.find(".")+1);
		GLenum type;
		if (suffix == "vs")
			type = GL_VERTEX_SHADER;
		else if (suffix == "fs")
			type = GL_FRAGMENT_SHADER;
		else if (suffix == "gs")
			type = GL_GEOMETRY_SHADER;

		std::string code;
		std::ifstream shaderFile;
		shaderFile.exceptions(std::ifstream::badbit);
		try {
			shaderFile.open(file);
			std::stringstream shaderStream;
			// Read file's buffer contents into streams
			shaderStream << shaderFile.rdbuf();
			// close file handlers
			shaderFile.close();
			// Convert stream into string
			code = shaderStream.str();
			assert(!code.empty());
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "failed to read " << file << std::endl;
		}

		GLuint shaderId;
		GLint success;
		const GLchar* shaderCode = code.c_str();
		GLchar infoLog[512];
		// Vertex Shader
		shaderId = glCreateShader(type);
		glShaderSource(shaderId, 1, &shaderCode, NULL);
		glCompileShader(shaderId);
		// Print compile errors if any
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		_shaders[file] = std::unique_ptr<GLuint, decltype(_shader_deleter)>(new GLuint(shaderId), _shader_deleter);
	}

	return *(_shaders[file]);
}
