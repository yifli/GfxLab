#pragma once

#include "common.h"


class ResourceManager {
public:
    static ResourceManager* GetInstance();
    void    LoadMesh(const std::string& file, MeshPtr& pMesh);
    GLuint  LoadTexture(const std::string& type, const std::string& path);
    GLuint  CreateTexture(GLenum target, GLint level, GLint internalFormat, GLsizei with, GLsizei height, GLint border, GLint format, GLenum type);
    GLuint  CreateRenderBuffer(GLenum target, GLenum internalFormat, GLsizei width, GLsizei height);
    GLuint  CreateProgram(std::vector<std::string>& shader_files);
    GLuint  GetScreenQuadVAO();
private:
    ResourceManager();
    GLuint  CreateShader(const std::string& file);

    static std::string SCREEN_QUAD;

    using VAOPtr          = std::unique_ptr<GLuint, std::function<void(GLuint*)>>;
    using TexObjPtr       = std::unique_ptr<GLuint, std::function<void(GLuint*)>>;
    using ShaderPtr       = std::unique_ptr<GLuint, std::function<void(GLuint*)>>;
    using ProgramPtr      = std::unique_ptr<GLuint, std::function<void(GLuint*)>>;
    using RenderBufferPtr = std::unique_ptr<GLuint, std::function<void(GLuint*)>>;

    std::unordered_map<std::string, VAOPtr>     _vertex_array_objs;
    std::unordered_map<std::string, TexObjPtr>  _texture_objs;
    std::unordered_map<std::string, ShaderPtr>  _shaders;
    std::unordered_map<std::string, ProgramPtr> _programs;
    std::vector<TexObjPtr>                      _fbo_textures;
    std::vector<RenderBufferPtr>                _fbo_renderbuffers;


    std::function<void(GLuint*)>               _vao_deleter;
    std::function<void(GLuint*)>               _texobj_deleter;
    std::function<void(GLuint*)>               _shader_deleter;
    std::function<void(GLuint*)>               _program_deleter;
    std::function<void(GLuint*)>               _renderbuffer_deleter;
};
