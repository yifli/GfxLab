#include "renderer.h"
#include "scene.h"
#include "geometry.h"
#include "camera.h"
#include "rendererfactory.h"
#include "renderpass.h"

Renderer::Renderer()
{
	RendererFactory::RegisterRenderer<Renderer>("Default");
}

void Renderer::Initialize()
{
    _globalCallback(_scene, _renderStates);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& rp : _renderpasses)
        rp->Render();
}

void Renderer::Resize(int width, int height)
{
	glViewport(0, 0, width, height);
	if (_scene != nullptr && _scene->GetCamera() != nullptr)
		_scene->GetCamera()->Resize(width, height);

}


