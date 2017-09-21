#pragma once

#include "common.h"
#include "resourcemanager.h"

#include <json/json.hpp>

using json = nlohmann::json;

class SceneParser {
public:
	SceneParser();
	void Parse(const char* file);

private:
	void        SetResourceLocations();
	WindowPtr   ParseWindow();
	ScenePtr    ParseScene();
	void        ParseCamera(ScenePtr, const json&);
	void        ParseGeometries(ScenePtr, const json&);
	void        ParseLights(ScenePtr, const json&);
	RendererPtr ParseRenderer();
	void        ParseTextures();
	void        ParsePrograms();
	void        ParseStateCallbacks();
    void        ParseRenderPass();
	void        SetRenderStates();

	void        ProcessFloatAttrib(const json&, const std::string&, const std::string&, bool, float&);
	void        ProcessIntAttrib(const json&, const std::string&, const std::string&, bool, int&);
	void        ProcessStringAttrib(const json&, const std::string&, const std::string&, bool, std::string&);
	void        ProcessNumberArrayAttrib(const json&, const std::string&, const std::string&, bool, std::vector<float>&);
	void        ProcessStringArrayAttrib(const json&, const std::string&, const std::string&, bool, std::vector<std::string>&);
	bool        ValidFolder(const std::string&);

	json                                         _j;
	int                                          _width, _height;
	ResourcePtr                                  _resmanager;
	RendererPtr                                  _renderer;
	std::unordered_map<std::string, GLuint>      _textures;
	std::unordered_map<std::string, GLuint>      _programs;
    std::unordered_map<std::string, GeometryPtr> _geometries;
	std::string                                  _gfxlab_root;
	std::string                                  _gfxlab_bin_dir;
	std::string                                  _gfxlab_shader_dir;
	std::string                                  _gfxlab_model_dir;
	std::string                                  _gfxlab_config_dir;
};