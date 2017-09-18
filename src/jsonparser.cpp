#include "jsonparser.h"
#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "light.h"
#include "mesh.h"
#include "rendererfactory.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>
#include <iostream>
#include <cctype>

static bool g_enable_logging = false;

#define LOGINFO(fmt, ...) \
	if (g_enable_logging) \
		fprintf(stdout, "[PARSER INFO]" fmt, ##__VA_ARGS__);

#define LOGERR(fmt, ...) \
{\
	fprintf(stderr, "[PARSER ERROR]" fmt, ##__VA_ARGS__);\
	assert(0);\
}

SceneParser::SceneParser()
{
	char* val = getenv("GFXLAB_ENABLE_PARSER_LOGGING");
	if (val && atoi(val) == 1)
		g_enable_logging = true;
	_resmanager = std::make_unique<ResourceManager>();
}

void SceneParser::SetResourceLocations()
{
	char* evar = getenv("GFXLAB_ROOT");
	_gfxlab_root = evar == NULL ?  "" : std::string(evar);

	evar = getenv("GFXLAB_MODEL_FOLDER");
	_gfxlab_model_dir = evar == NULL ? "" : std::string(evar);

	evar = getenv("GFXLAB_SHADER_FOLDER");
	_gfxlab_shader_dir = evar == NULL ? "" : std::string(evar);

	evar = getenv("GFXLAB_BIN_FOLDER");
	_gfxlab_bin_dir = evar == NULL ? "" : std::string(evar);

	evar = getenv("GFXLAB_CONFIG_FOLDER");
	_gfxlab_config_dir = evar == NULL ? "" : std::string(evar);

	if (_gfxlab_root.empty())
		_gfxlab_root = "./";
	else if (!ValidFolder(_gfxlab_root)) {
		std::cerr << "GFXLAB_ROOT " << _gfxlab_root << " does not exist\n";
		assert(0);
	}

	if (_gfxlab_model_dir.empty())
		_gfxlab_model_dir = _gfxlab_root + "/models/";
	if (!ValidFolder(_gfxlab_model_dir)) {
			std::cerr << "GFXLAB_MODEL_FOLDER " << _gfxlab_model_dir << " dos not exist\n";
			assert(0);
	}

	if (_gfxlab_shader_dir.empty())
		_gfxlab_shader_dir = _gfxlab_root + "/shaders/";
	if (!ValidFolder(_gfxlab_shader_dir)) {
		std::cerr << "GFXLAB_SHADER_FOLDER " << _gfxlab_shader_dir << " dos not exist\n";
		assert(0);
	}

	if (_gfxlab_bin_dir.empty())
		_gfxlab_bin_dir = _gfxlab_root + "/bin/";
	if (!ValidFolder(_gfxlab_bin_dir)) {
		std::cerr << "GFXLAB_BIN_FOLDER " << _gfxlab_bin_dir << " dos not exist\n";
		assert(0);
	}

	if (_gfxlab_config_dir.empty())
		_gfxlab_config_dir = _gfxlab_root + "/configs/";
	if (!ValidFolder(_gfxlab_model_dir)) {
		std::cerr << "GFXLAB_CONFIG_FOLDER " << _gfxlab_config_dir << " dos not exist\n";
		assert(0);
	}
}

void SceneParser::Parse(const char* file)
{
	SetResourceLocations();

	std::ifstream input(_gfxlab_config_dir+"/"+file);
	if (!input.is_open()) {
		std::cout << "failed to open " << file << std::endl;
		std::exit(-1);
	}
	try {
		input >> _j;

		LOGINFO("Parsing attribute 'Window'...\n");
		auto window = ParseWindow();
		_renderer = ParseRenderer();
		ParseTextures();
		ParsePrograms();
		SetRenderStates();
		auto scene = ParseScene();
		if (scene != nullptr)
			_renderer->SetScene(scene);
		ParseStateCallbacks();
		window->SetRenderer(_renderer);
		window->Display();
	}
	catch (std::exception& e) {
		LOGERR("%s\n", e.what());
	}
}

WindowPtr SceneParser::ParseWindow()
{
	std::string title;
	_width = Window::WIDTH;
	_height = Window::HEIGHT;
	title = Window::TITLE;

	if (_j.find("Window") != _j.end() ) {
		if (_j["Window"].is_object()) {
			auto settings = _j["Window"];
			ProcessIntAttrib(_j["Window"], "width", "Window.width", false, _width);
			ProcessIntAttrib(_j["Window"], "height", "Window.height", false, _height);
			ProcessStringAttrib(_j["Window"], "title", "Window.title", false, title);
		}
		else {
			LOGERR("Expects a JSON object for the attribute 'Window'\n");
		}
	}

	return std::unique_ptr<Window>(Window::Create(title.c_str(), _width, _height));
}

ScenePtr SceneParser::ParseScene()
{
	ScenePtr scene = std::shared_ptr<Scene>(nullptr);
	if (_j.find("Scene") != _j.end()) {
		if (_j["Scene"].is_object()) {
			LOGINFO("Parsing attribute 'Scene'...\n");
			auto scene_object = _j["Scene"];
			scene.reset(new Scene());
			if (scene_object.find("Camera") != scene_object.end()) {
				if (scene_object["Camera"].is_object())
					ParseCamera(scene, scene_object["Camera"]);
				else
					LOGERR("Expects a JSON object for the attribute Scene.Camera\n");
			}

			
			std::string progName;
			ProcessStringAttrib(scene_object, "DefaultProgram", "Scene.DefaultProgram", true, progName);
			if (_programs.find(progName) == _programs.end())
				LOGERR("Failed to find program %s\n", progName.c_str());
			_default_program = _programs[progName];
		
				
			if (scene_object.find("Geometries") != scene_object.end()) {
				if (scene_object["Geometries"].is_array())
					ParseGeometries(scene, scene_object["Geometries"]);
				else
					LOGERR("Expects a JSON array for the attribute Scene.Geometries\n");
			}

			if (scene_object.find("Lights") != scene_object.end()) {
				if (scene_object["Lights"].is_array())
					ParseLights(scene, scene_object["Lights"]);
				else
					LOGERR("Expects a JSON array for the attribute Scene.Lights\n");
			}
		}
		else {
			LOGERR("Expects a JSON object for 'Scene'\n");
		}
	}

	return scene;
}

void SceneParser::ParseCamera(ScenePtr scene, const json& cam_settings)
{
	LOGINFO("Parsing attribute 'Scene.Camera'...\n");

	std::vector<float> pos, focus, up;
	ProcessNumberArrayAttrib(cam_settings, "pos", "Scene.Camera.pos", true, pos);
	ProcessNumberArrayAttrib(cam_settings, "focus", "Scene.Camera.focus", true, focus);
	ProcessNumberArrayAttrib(cam_settings, "up", "Scene.Camera.up", true, up);

	CameraPtr cam = std::make_shared<Camera>(0, 0, _width, _height);
	cam->LookAt(glm::vec3(pos[0], pos[1], pos[2]), glm::vec3(focus[0], focus[1], focus[2]), glm::vec3(up[0], up[1], up[2]));
	scene->SetCamera(cam);
}

void SceneParser::ParseGeometries(ScenePtr scene, const json& geom_settings)
{
	LOGINFO("Parsing attribute 'Scene.Geometries'...\n");
	
	std::string attib_full_name, source, program, id;
	int geom_id = 0;
	for (auto& geom : geom_settings) {
		GeometryPtr mesh = std::shared_ptr<Geometry>(new Mesh);
		attib_full_name = "Scene.Geometries[" + std::to_string(geom_id) + "].";
		ProcessStringAttrib(geom, "name", attib_full_name + "name", true, id);
		mesh->SetName(id);

		ProcessStringAttrib(geom, "source", attib_full_name+"source", true, source);
		source = _gfxlab_model_dir + "/" + source;
		_resmanager->LoadMesh(source, std::static_pointer_cast<Mesh>(mesh));

		ProcessStringAttrib(geom, "program", attib_full_name + "program", false, program);
		if (program.empty()) {
			assert(_default_program > 0);
			mesh->SetShaderProgram(_default_program);
		}
		else {
			assert(_programs.find(program) != _programs.end() && _programs[program] > 0);
			mesh->SetShaderProgram(_programs[program]);
		}
		scene->AddGeometry(mesh);
	}
}

void SceneParser::ParseLights(ScenePtr scene, const json& light_settings)
{
	LOGINFO("Parsing attribute 'Scene.Lights'...\n");

	std::string light_type;
	std::vector<float> pos, dir, ambient, diffuse, specular;
	float constantAttn, linearAttn, quadAttn;
	constantAttn = linearAttn = quadAttn = -1;
	int light_id = 0;
	for (auto& light : light_settings) {
		LightPtr l = std::make_shared<Light>();
		std::string attrib_full_name = "Scene.Lights[" + std::to_string(light_id) + "].";
		ProcessStringAttrib(light, "type", attrib_full_name + "type", true, light_type);

		if (light_type == "Point") {
			ProcessNumberArrayAttrib(light, "pos", attrib_full_name + "pos", true, pos);
			l->SetType(Light::POINT);
			l->SetPosition(glm::vec3(pos[0], pos[1], pos[2]));
		}
		else {
			ProcessNumberArrayAttrib(light, "dir", attrib_full_name + "dir", true, dir);
			l->SetType(Light::DIRECTION);
			l->SetDirection(glm::vec3(dir[0], dir[1], dir[2]));
		}

		ProcessNumberArrayAttrib(light, "ambient", attrib_full_name + "ambient", false, ambient);
		if (!ambient.empty())
			l->SetAmbient(glm::vec3(ambient[0], ambient[1], ambient[2]));
		ProcessNumberArrayAttrib(light, "diffuse", attrib_full_name + "diffuse", false, diffuse);
		if (!diffuse.empty())
			l->SetDiffuse(glm::vec3(diffuse[0], diffuse[1], diffuse[2]));
		ProcessNumberArrayAttrib(light, "specular", attrib_full_name + "specular", false, specular);
		if (!specular.empty())
			l->SetSpecular(glm::vec3(specular[0], specular[1], specular[2]));
		ProcessFloatAttrib(light, "constant_atten", attrib_full_name + "constant_atten", false, constantAttn);
		if (constantAttn >= 0)
			l->SetConstantAtten(constantAttn);
		ProcessFloatAttrib(light, "linear_atten", attrib_full_name + "linear_atten", false, linearAttn);
		if (linearAttn >= 0)
			l->SetLinearAtten(linearAttn);
		ProcessFloatAttrib(light, "quadratic_atten", attrib_full_name + "quadratic_atten", false, quadAttn);
		if (quadAttn >= 0)
			l->SetQuadraticAtten(quadAttn);

		if (constantAttn == 0 && linearAttn == 0 && quadAttn == 0)
			LOGERR("One of the attenulation factors must be greater than 0");

		++light_id;

		scene->AddLight(l);
		
	}

}

RendererPtr SceneParser::ParseRenderer()
{
	LOGINFO("Parsing attribute 'Renderer'...\n");

	std::string renderer_name = "default";
	if (_j.find("Renderer") == _j.end()) {
		LOGINFO("Use default renderer since Renderer is not set.\n");
	}
	else {
		ProcessStringAttrib(_j, "Renderer", "Renderer", true, renderer_name);
	}
	
	return RendererFactory::MakeRenderer(renderer_name);
}

void SceneParser::ParseTextures()
{
	LOGINFO("Parsing attribute 'Textures'...\n");

	if (_j.find("Textures") != _j.end()) {
		if (_j["Textures"].is_array()) {
			auto textures = _j["Textures"].get<std::vector<json>>();
			int tex_id = 0;
			std::string tex_name, tex_type, tex_file;
			for (auto& tex : textures) {
				std::string attrib_full_name = "Textures[" + std::to_string(tex_id) + "].";
				ProcessStringAttrib(tex, "name", attrib_full_name+"name", true, tex_name);
				ProcessStringAttrib(tex, "type", attrib_full_name + "type", true, tex_type);
				ProcessStringAttrib(tex, "source", attrib_full_name + "source", true, tex_file);
				++tex_id;

				_textures[tex_name] =  _resmanager->LoadTexture(tex_type, tex_file);
			}
		}
		else {
			LOGERR("Expects a JSON array for the attribute 'Textures'\n");
		}
	}
}

void SceneParser::ParsePrograms()
{
	LOGINFO("Parsing attribute 'Programs'...\n");

	if (_j.find("Programs") != _j.end()) {
		if (_j["Programs"].is_array()) {
			auto programs = _j["Programs"];
			std::string prog_name, shaders;
			int prog_id = 0;
			for (auto& prog : programs) {
				std::string attrib_full_name = "Programs[" + std::to_string(prog_id) + "].";
				ProcessStringAttrib(prog, "name", attrib_full_name + "name", true, prog_name);
				ProcessStringAttrib(prog, "shaders", attrib_full_name + "shaders", true, shaders);
				++prog_id;

				std::vector<std::string> shader_files;
				size_t prev = 0, next;
				while ((next = shaders.find_first_of(";", prev)) != std::string::npos) {
					shader_files.push_back(_gfxlab_shader_dir + "/" + shaders.substr(prev, next - prev));
					next++;
					while (std::isspace(shaders[next]))
						next++;
					prev = next;
				}
				shader_files.push_back(_gfxlab_shader_dir + "/" + shaders.substr(prev));
				_programs[prog_name] = _resmanager->CreateProgram(shader_files);
			}
		}
		else {
			LOGERR("Expects a JSON array for the attribute 'Programs'\n");
		}
	}
}

void SceneParser::ParseStateCallbacks()
{
	if (_j.find("SetStateCallbacks") != _j.end()) {
		LOGINFO("Parsing attribute 'SetStateCallbacks'...\n");
		
		auto callback_settings = _j["SetStateCallbacks"];
		std::string library;
		ProcessStringAttrib(callback_settings, "library", "SetStateCallbacks.library", true, library);
		library = _gfxlab_bin_dir + "/" + library;

		std::vector<std::string> global_state_cbs = { "SetGlobalStates"      };
		std::vector<std::string> per_frame_cbs    = { "SetPerFrameStates"    };
		std::vector<std::string> per_program_cbs  = { "SetPerProgramStates"  };
		std::vector<std::string> per_geom_cbs     = { "SetPerGeometryStates" };

		ProcessStringArrayAttrib(callback_settings, "global_state_cbs", "SetStateCallbacks.global_state_cbs", false, global_state_cbs);
		ProcessStringArrayAttrib(callback_settings, "per_frame_cbs", "SetStateCallbacks.per_frame_cbs", false, per_frame_cbs);
		ProcessStringArrayAttrib(callback_settings, "per_program_cbs", "SetStateCallbacks.per_program_cbs", false, per_program_cbs);
		ProcessStringArrayAttrib(callback_settings, "per_geom_cbs", "SetStateCallbacks.per_geom_cbs", false, per_geom_cbs);

		auto lib = LoadLibrary(library.c_str());
		if (lib == nullptr) {
			LOGERR("Failed to load library %s: %d\n", library.c_str(), GetLastError());
		}
		else {
			for (auto& str : global_state_cbs) {
				auto cb = GetProcAddress(lib, str.c_str());
				if (cb != nullptr)
					_renderer->AddGlobalSetStateCallbacks(reinterpret_cast<void(*)(const ScenePtr&, RenderStates&)>(cb));
			}

			for (auto& str : per_frame_cbs) {
				auto cb = GetProcAddress(lib, str.c_str());
				if (cb != nullptr)
					_renderer->AddFrameSetStateCallbacks(reinterpret_cast<void(*)(const ScenePtr&, RenderStates&)>(cb));
			}

			for (auto& str : per_program_cbs) {
				auto cb = GetProcAddress(lib, str.c_str());
				if (cb != nullptr)
					_renderer->AddProgramSetStateCallbacks(reinterpret_cast<void(*)(const ScenePtr&, GLuint, RenderStates&, ProgramRenderStates&)>(cb));
			}

			for (auto& str : per_geom_cbs) {
				auto cb = GetProcAddress(lib, str.c_str());
				if (cb != nullptr)
					_renderer->AddGeometrySetStateCallbacks(reinterpret_cast<void(*)(const GeometryPtr&, ProgramRenderStates&)>(cb));
			}
		}
		
	}
}

void SceneParser::SetRenderStates()
{
	for (auto& kv : _programs)
		_renderer->AddShaderProgram(kv.first, kv.second);
}



void SceneParser::ProcessIntAttrib(const json& j, const std::string& name, const std::string& full_name, bool required, int& result)
{
	if (j.find(name) != j.end()) {
		if (j[name].is_number_integer()) {
			result = j[name].get<int>();
			LOGINFO("%s is set to %d\n", full_name.c_str(), result);
		}
		else {
			LOGERR("Expects an int for the attribute %s\n", full_name.c_str());
		}
	}
	else {
		if (required)
			LOGERR("Missing attribute '%s'\n", full_name.c_str());
	}
}


void SceneParser::ProcessFloatAttrib(const json& j, const std::string& name, const std::string& full_name, bool required, float& result)

{
	if (j.find(name) != j.end()) {
		if (j[name].is_number_float()) {
			result = j[name].get<float>();
			LOGINFO("%s is set to %f\n", full_name.c_str(), result);
		}
		else {
			LOGERR("Expects a float for the attribute %s\n", full_name.c_str());
		}
	}
	else {
		if (required)
			LOGERR("Missing attribute '%s'\n", full_name.c_str());
	}
}

void SceneParser::ProcessStringAttrib(const json& j, const std::string& name, const std::string& full_name, bool required, std::string& result)
{
	if (j.find(name) != j.end()) {
		if (j[name].is_string()) {
			result = j[name].get<std::string>();
			LOGINFO("%s is set to %s\n", full_name.c_str(), result.c_str());
		}
		else {
			LOGERR("Expects a string for the attribute %s\n", full_name.c_str());
		}
	}
	else {
		if (required)
			LOGERR("Missing attribute '%s'\n", full_name.c_str());
	}
}

void SceneParser::ProcessNumberArrayAttrib(const json& j, const std::string& name, const std::string& full_name, bool required, std::vector<float>& result)
{
	if (j.find(name) != j.end()) {
		if (j[name].is_array()) {
			result = j[name].get<std::vector<float>>();
			std::string str_result;
			for (auto n : result)
				str_result += std::to_string(n) + " ";
			LOGINFO("%s is set to [ %s]\n", full_name.c_str(), str_result.c_str());
		}
		else {
			LOGERR("Expects a JSON array for the attribute %s\n", full_name.c_str());
		}
	}
	else {
		if (required)
			LOGERR("Missing attribute '%s'\n", full_name.c_str());
	}
}

void SceneParser::ProcessStringArrayAttrib(const json& j, const std::string& name, const std::string& full_name, bool required, std::vector<std::string>& result)
{
	if (j.find(name) != j.end()) {
		if (j[name].is_array()) {
			result = j[name].get<std::vector<std::string>>();
			std::string str_result;
			for (auto& str : result)
				str_result += str + " ";
			LOGINFO("%s is set to [ %s]\n", full_name.c_str(), str_result.c_str());
		}
		else {
			LOGERR("Expects a JSON array for the attribute %s\n", full_name.c_str());
		}
	}
	else {
		if (required)
			LOGERR("Missing attribute '%s'\n", full_name.c_str());
	}
}

bool SceneParser::ValidFolder(const std::string& folder)
{
	struct stat sb;
	return stat(folder.c_str(), &sb) == 0 && (sb.st_mode & S_IFDIR);
}

