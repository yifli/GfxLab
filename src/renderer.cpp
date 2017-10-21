#include "renderer.h"
#include "scene.h"
#include "geometry.h"
#include "camera.h"
#include "rendererfactory.h"
#include "renderpass.h"

#include <unordered_set>

Renderer::Renderer()
    :_globalCallback(nullptr),
    _perFrameCallback(nullptr),
    _perProgramCallback(nullptr),
    _perGeometryCallback(nullptr)
{
    RendererFactory::RegisterRenderer<Renderer>("Default");
}

void Renderer::Initialize()
{
    if (_globalCallback)
        _globalCallback(_scene, _renderStates);
}

void Renderer::Render()
{
    std::unordered_set<GLuint> fbos;
    bool needs_clear;
    GLuint fbo;
    for (auto& rp : _renderpasses) {
        fbo = rp->GetFBO();
        needs_clear = fbos.find(fbo) == fbos.end();
        rp->Render(needs_clear);
        fbos.insert(fbo);
    }
}

void Renderer::Resize(int width, int height)
{
    glViewport(0, 0, width, height);
    if (_scene != nullptr && _scene->GetCamera() != nullptr)
        _scene->GetCamera()->Resize(width, height);

}
