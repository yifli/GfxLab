#include "resourcemanager.h"
#include "mesh.h"

#include <SOIL.h>

#include <cstdlib>
#include <cassert>

std::string ResourceManager::SCREEN_QUAD = "screen_quad";

ResourceManager* ResourceManager::GetInstance()
{
    static ResourceManager manager;
    return &manager;
}

ResourceManager::ResourceManager()
{
    _vao_deleter          = [](GLuint* id) { glDeleteVertexArrays(1, id); };
    _texobj_deleter       = [](GLuint* id) { glDeleteTextures(1, id); };
    _program_deleter      = [](GLuint* id) { glDeleteProgram(*id); };
    _shader_deleter       = [](GLuint* id) { glDeleteShader(*id); };
    _renderbuffer_deleter = [](GLuint* id) { glDeleteRenderbuffers(1, id); };
}


void ResourceManager::LoadMesh(const std::string& file, MeshPtr& pMesh)
{
    auto& mesh = pMesh->GetMeshObj();
    mesh.request_vertex_normals();
    mesh.request_vertex_texcoords2D();
    mesh.request_vertex_colors();
    mesh.request_face_colors();
    mesh.request_vertex_status();
    mesh.request_edge_status();
    mesh.request_face_status();
    mesh.request_halfedge_status();

    OpenMesh::IO::Options opt;
    opt += OpenMesh::IO::Options::VertexNormal;
    opt += OpenMesh::IO::Options::VertexTexCoord;
    opt += OpenMesh::IO::Options::VertexColor;
    opt += OpenMesh::IO::Options::FaceColor;
    assert(OpenMesh::IO::read_mesh(mesh, file, opt));

    pMesh->DeleteIsolatedVerts();

    mesh.release_vertex_status();
    mesh.release_edge_status();
    mesh.release_face_status();
    mesh.release_halfedge_status();


    mesh.request_face_normals();
    mesh.update_normals();
    mesh.release_face_normals();

    if (!opt.check(OpenMesh::IO::Options::VertexTexCoord))
        mesh.release_vertex_texcoords2D();

    if (!opt.check(OpenMesh::IO::Options::VertexColor))
        mesh.release_vertex_colors();

    if (opt.check(OpenMesh::IO::Options::FaceColor))
        pMesh->EnablePerFaceShading(true);
    else
        mesh.release_face_colors();

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

GLuint  ResourceManager::CreateTexture(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLint format, GLenum type)
{
    GLuint texobj;
    glGenTextures(1, &texobj);
    auto pTex = std::unique_ptr<GLuint, decltype(_texobj_deleter)>(new GLuint(texobj), _texobj_deleter);
    _fbo_textures.push_back(std::move(pTex));

    glBindTexture(GL_TEXTURE_2D, texobj);
    glTexImage2D(target, level, internalFormat, width, height, 0, format, type, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texobj;
}

GLuint  ResourceManager::CreateRenderBuffer(GLenum target, GLenum internalFormat, GLsizei width, GLsizei height)
{
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    auto pRbo = std::unique_ptr<GLuint, decltype(_renderbuffer_deleter)>(new GLuint(rbo), _renderbuffer_deleter);
    _fbo_renderbuffers.push_back(std::move(pRbo));

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(target, internalFormat, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    return rbo;
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
            
            glDeleteProgram(program);
            program = 0;
        }
        else {
            _programs[str] = std::unique_ptr<GLuint, decltype(_program_deleter)>(new GLuint(program), _program_deleter);
        }

        return program;
    }
    else {
        return *(_programs[str]);
    }
}

GLuint ResourceManager::CreateShader(const std::string& file)
{
    if (_shaders.find(file) == _shaders.end()) {
        std::string suffix = file.substr(file.find_last_of(".")+1);
        GLenum type;
        if (suffix == "vs")
            type = GL_VERTEX_SHADER;
        else if (suffix == "fs")
            type = GL_FRAGMENT_SHADER;
        else if (suffix == "gs")
            type = GL_GEOMETRY_SHADER;
        else {
            fprintf(stderr, "unsupported shader file suffix %s\n", suffix.c_str());
            assert(0);
        }

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
            std::cout << file << ": " << infoLog << std::endl;
        }

        _shaders[file] = std::unique_ptr<GLuint, decltype(_shader_deleter)>(new GLuint(shaderId), _shader_deleter);
    }

    return *(_shaders[file]);
}

GLuint  ResourceManager::GetScreenQuadVAO()
{
    if (_vertex_array_objs.find(SCREEN_QUAD) == _vertex_array_objs.end()) {
        float quad_vertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
        };

        GLuint vao, vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);

        _vertex_array_objs[SCREEN_QUAD] = std::unique_ptr<GLuint, decltype(_vao_deleter)>(new GLuint(vao), _vao_deleter);
        return vao;
    }
    else {
        return *(_vertex_array_objs[SCREEN_QUAD]);
    }
}
