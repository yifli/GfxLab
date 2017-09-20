#pragma once

#include "common.h"
#include "scene.h"
#include "renderstatecallbacks.h"




class Renderer {
    friend class RenderPass;
public:
	Renderer();

	void      SetScene(ScenePtr scene)                                    { _scene = scene; }
	void      SetGlobalSetStateCallback(SetGlobalStateCallback cb)        { _globalCallback = cb; }
	void      SetFrameSetStateCallback(SetPerFrameStateCallback cb)       { _perFrameCallback = cb; }
	void      SetProgramSetStateCallback(SetPerProgramStateCallback cb)   { _perProgramCallback = cb; }
	void      SetGeometrySetStateCallback(SetPerGeometryStateCallback cb) { _perGeometryCallback = cb; }
	void      AddShaderProgram(const std::string name, GLuint id)
	{
		_renderStates.programs[name] = id;
		_renderStates.reverse_program_lookup[id] = name;
	}
	CameraPtr GetCamera()
	{
		if (_scene != nullptr)
			return _scene->GetCamera();
		else
			return nullptr;
	}


	virtual void      Initialize();
	virtual void      Render();
	virtual void      Resize(int width, int height);
	virtual void      OnKeyPressed(int key, int scancode, int action, int mods) {}


protected:
	ScenePtr                                              _scene;
	std::unordered_map<GLuint, std::vector<GeometryPtr>>  _geometriesByProgram;
	SetGlobalStateCallback                                _globalCallback;
	SetPerFrameStateCallback                              _perFrameCallback;
	SetPerProgramStateCallback                            _perProgramCallback;
	SetPerGeometryStateCallback                           _perGeometryCallback;
	RenderStates                                          _renderStates;
};