#include "renderer.h"
#include "scene.h"
#include "geometry.h"
#include "camera.h"
#include "rendererfactory.h"

Renderer::Renderer()
{
	RendererFactory::RegisterRenderer<Renderer>("Default");
}

void Renderer::Initialize()
{
	if (_scene != nullptr) {
		for (auto& g : _scene->GetGeometries())
			_geometriesByProgram[g->GetShaderProgram()].push_back(g);
	}

	for (auto& cb : _globalCallbacks) {
		cb(_scene, _renderStates);
	}
}

void Renderer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& cb : _perFrameCallbacks)
		cb(_scene, _renderStates);

	for (auto& kv : _geometriesByProgram) {
		glUseProgram(kv.first);
		for (auto& cb : _perProgramCallbacks)
			cb(_scene, kv.first, _renderStates, _renderStates.program_states[kv.first]);

		for (auto& g : kv.second) {
			for (auto& cb : _perGeometryCallbacks)
				cb(g, _renderStates.program_states[kv.first]);
			g->Render();
		}

		glUseProgram(0);
	}

}

void Renderer::Resize(int width, int height)
{
	glViewport(0, 0, width, height);
	if (_scene != nullptr && _scene->GetCamera() != nullptr)
		_scene->GetCamera()->Resize(width, height);

}


