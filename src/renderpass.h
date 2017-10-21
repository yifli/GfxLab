#pragma once
#include "common.h"

class RenderPass {
public:
    RenderPass(RendererPtr renderer);

    ~RenderPass();

    GLuint    GetFBO() const { return _fbo; }

    void      SetProgramForGeometries(GLuint prog_id, const std::vector<GeometryPtr>& geoms);

    void      SetProgram(GLuint prog_id);

    bool      CreateFBO(std::vector<std::pair<GLuint, GLenum>>& color_attachments,
                        std::pair<GLuint, GLenum>& depth_attachment,
                        std::pair<GLuint, GLenum>& stencil_attachment,
                        std::pair<GLuint, GLenum>& ds_attachment);

    void      SetInputTextures(const std::vector<GLuint>& textures);

    void      Render(bool needs_clear);

    void      SetDisplayImage(GLuint texture);

private:
    void      SetProgramStates();

    void      BlitTextureToSceen();

    void      RenderScene(bool needs_clear);

    RendererPtr                 _renderer;
    GLuint                      _prog;
    std::vector<GeometryPtr>    _geometries;
    std::vector<GLuint>         _inputTextures;
    GLuint                      _fbo;
    bool                        _useColorBuffer;
    bool                        _useDepthBuffer;
    bool                        _useStencilBuffer;
    bool                        _isBlit;
    GLuint                      _textureToBlit;
};