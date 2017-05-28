#pragma once

#include "common.h"
#include "scene.h"
#include "renderstatecallbacks.h"




class Renderer {
public:
	void      SetScene(ScenePtr scene)                                     { _scene = scene; }
	void      Initialize();
	void      Render();
	void      Resize(int width, int height);
	CameraPtr GetCamera();
	void      AddGlobalSetStateCallbacks(SetGlobalStateCallback cb)        { _globalCallbacks.push_back(cb); }
	void      AddFrameSetStateCallbacks(SetPerFrameStateCallback cb)       { _perFrameCallbacks.push_back(cb); }
	void      AddProgramSetStateCallbacks(SetPerProgramStateCallback cb)   { _perProgramCallbacks.push_back(cb); }
	void      AddGeometrySetStateCallbacks(SetPerGeometryStateCallback cb) { _perGeometryCallbacks.push_back(cb); }
	void      AddShaderProgram(const std::string name, GLuint id)          { _renderStates.programs[name] = id;
	                                                                         _renderStates.reverse_program_lookup[id] = name;
	                                                                       }

private:
	ScenePtr                                              _scene;
	std::unordered_map<GLuint, std::vector<GeometryPtr>>  _geometriesByProgram;
	std::vector<SetGlobalStateCallback>                   _globalCallbacks;
	std::vector<SetPerFrameStateCallback>                 _perFrameCallbacks;
	std::vector<SetPerProgramStateCallback>               _perProgramCallbacks;
	std::vector<SetPerGeometryStateCallback>              _perGeometryCallbacks;
	RenderStates                                          _renderStates;
};