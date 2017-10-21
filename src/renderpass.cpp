#include "renderpass.h"
#include "renderstatecallbacks.h"
#include "renderer.h"
#include "geometry.h"
#include "resourcemanager.h"

RenderPass::RenderPass(RendererPtr renderer)
    : _renderer(renderer),
    _fbo(0),
    _useColorBuffer(true),
    _useDepthBuffer(true),
    _useStencilBuffer(false),
    _isBlit(false)
{
}

RenderPass::~RenderPass()
{
    glDeleteFramebuffers(1, &_fbo);
}

void RenderPass::SetProgramForGeometries(GLuint prog_id, const std::vector<GeometryPtr>& geoms)
{
    _prog = prog_id;
    _geometries = geoms;
}

void RenderPass::SetProgram(GLuint prog_id)
{
    _prog = prog_id;
    _geometries = _renderer->_scene->GetGeometries();
}

void RenderPass::SetProgramStates()
{
    auto& prog_cb = _renderer->_perProgramCallback;
    if (prog_cb)
        prog_cb(_renderer->_scene, _prog, _renderer->_renderStates, _renderer->_renderStates.program_states[_prog]);
 }


void RenderPass::Render(bool needs_clear)
{
    if (_isBlit)
        BlitTextureToSceen();
    else
        RenderScene(needs_clear);
}

bool RenderPass::CreateFBO(std::vector<std::pair<GLuint, GLenum>>& color_attachments,
    std::pair<GLuint, GLenum>& depth_attachment,
    std::pair<GLuint, GLenum>& stencil_attachment,
    std::pair<GLuint, GLenum>& ds_attachment)
{
    bool status = true;

    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    GLuint id;
    GLenum type;
    for (size_t i = 0; i < color_attachments.size(); i++) {
        id = color_attachments[i].first;
        assert(id != 0);
        type = color_attachments[i].second;

        if (type == GL_TEXTURE_2D)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, type, id, 0);
        else
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, type, id);
    }
    _useColorBuffer = !color_attachments.empty();

    id = depth_attachment.first;
    if (id != 0) {
        type = depth_attachment.second;
        if (type == GL_TEXTURE_2D)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, type, id, 0);
        else
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, type, id);
    }

    id = stencil_attachment.first;
    if (id != 0) {
        type = stencil_attachment.second;
        if (type == GL_TEXTURE_2D)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, type, id, 0);
        else
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, type, id);
    }

    id = ds_attachment.first;
    if (id != 0) {
        type = ds_attachment.second;
        if (type == GL_TEXTURE_2D)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, type, id, 0);
        else
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, type, id);
    }

    if (!_useColorBuffer) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Failed to create FBO: " << glewGetErrorString(glGetError()) << std::endl;
        status = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return status;
}

void RenderPass::SetInputTextures(const std::vector<GLuint>& textures)
{ 
    _inputTextures = textures;
}

void RenderPass::SetDisplayImage(GLuint texture)
{
    _isBlit = true;
    _textureToBlit = texture;
}

void RenderPass::BlitTextureToSceen()
{
    static GLuint vao = ResourceManager::GetInstance()->GetScreenQuadVAO();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(_prog);
    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, _textureToBlit);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);
}

void RenderPass::RenderScene(bool needs_clear)
{
    for (size_t i = 0; i < _inputTextures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, _inputTextures[i]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    GLbitfield mask = 0;
    if (_useColorBuffer) {
        mask |= GL_COLOR_BUFFER_BIT;
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
    }
    if (_useDepthBuffer) {
        mask |= GL_DEPTH_BUFFER_BIT;
        glEnable(GL_DEPTH_TEST);
    }
    if (_useStencilBuffer) {
        mask |= GL_STENCIL_BUFFER_BIT;
        glEnable(GL_STENCIL_TEST);
    }

    if (needs_clear)
        glClear(mask);

    glUseProgram(_prog);

    SetProgramStates();

    for (auto& g : _geometries) {
        if (_renderer->_perGeometryCallback)
            _renderer->_perGeometryCallback(g, _renderer->_renderStates.program_states[_prog]);
        g->Render();
    }

    glUseProgram(0);

    if (_useDepthBuffer)
        glDisable(GL_DEPTH_TEST);

    if (_useStencilBuffer)
        glDisable(GL_STENCIL_TEST);

}