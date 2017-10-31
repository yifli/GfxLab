#pragma once

#include "common.h"
#include "resourcemanager.h"

#include <json/json.hpp>

using json = nlohmann::json;
using FBOInfo = std::pair<GLuint, GLenum>;


class SceneParser {
public:
    SceneParser();
    WindowPtr   Parse(const char* file);

private:
    void        SetResourceLocations();
    WindowPtr   ParseWindow();
    ScenePtr    ParseScene();
    void        ParseCamera(ScenePtr, const json&);
    void        ParseGeometries(ScenePtr, const json&);
    void        ParseGeometryInstanceData(const json&, GeometryPtr, int);
    void        ParseGeometryInstanceDataHelper(const json&, GeometryPtr, const std::string&, void**, size_t&, size_t&);
    void        ParseGeometryTransformation(const json&, glm::mat4&, const std::string&);
    void        ParseLights(ScenePtr, const json&);
    RendererPtr ParseRenderer();
    void        ParseStateCallbacks();
    void        ParseRenderPasses();
    void        ParseSingleRenderPass(const json&, int);
    GLuint      ParseProgramInRenderPass(const json&, const std::string&);
    void        ParseGeometriesInRenderPass(const json&, const std::string, std::vector<GeometryPtr>&);
    void        ParseFBOAttachmentsInRenderPass(const json&,
                                    std::vector<std::pair<GLuint, GLenum>>&,
                                    std::pair<GLuint, GLenum>&,
                                    std::pair<GLuint, GLenum>&,
                                    std::pair<GLuint, GLenum>&,
                                    const std::string&);
    void        RecordFBOInfoForRenderPass(GLuint fbo,
                                           std::vector<std::pair<GLuint, GLenum>>&,
                                           std::pair<GLuint, GLenum>&,
                                           std::pair<GLuint, GLenum>&,
                                           std::pair<GLuint, GLenum>&);

    void        ParseInputTextures(const json&, int rp, std::vector<GLuint>&);
    GLuint      GetTextureObj(const std::string&);
    FBOInfo     CreateFBOAttachment(const json&, const std::string&, const std::string&);



    void        ProcessFloatAttrib(const json&, const std::string&, const std::string&, bool, float&);
    void        ProcessIntAttrib(const json&, const std::string&, const std::string&, bool, int&);
    void        ProcessStringAttrib(const json&, const std::string&, const std::string&, bool, std::string&);
    void        ProcessNumberArrayAttrib(const json&, const std::string&, const std::string&, bool, std::vector<float>&);
    void        ProcessStringArrayAttrib(const json&, const std::string&, const std::string&, bool, std::vector<std::string>&);
    
    bool        ValidFolder(const std::string&);
    void        CollectShaderFiles(const std::string&, std::vector<std::string>&);

    json                                         _j;
    int                                          _width, _height;
    RendererPtr                                  _renderer;
    std::unordered_map<std::string, GeometryPtr> _geometries;
    std::unordered_map<std::string, GLuint>      _programs;

    struct FBOAttachments {
        std::vector<GLuint> color;
        GLuint depth;
        GLuint stencil;
        GLuint depth_stencil;
    };
    std::unordered_map<int, FBOAttachments>      _fboAttachments;
    std::string                                  _gfxlab_root;
    std::string                                  _gfxlab_bin_dir;
    std::string                                  _gfxlab_shader_dir;
    std::string                                  _gfxlab_model_dir;
    std::string                                  _gfxlab_config_dir;
    std::string                                  _gfxlab_texture_dir;
};