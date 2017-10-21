#include "jsonparser.h"
#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "light.h"
#include "mesh.h"
#include "rendererfactory.h"
#include "renderpass.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>
#include <iostream>
#include <cctype>
#include <regex>

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

    evar = getenv("GFXLAB_TEXTURE_FOLDER");
    _gfxlab_texture_dir = evar == NULL ? "" : std::string(evar);

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
    if (!ValidFolder(_gfxlab_config_dir)) {
        std::cerr << "GFXLAB_CONFIG_FOLDER " << _gfxlab_config_dir << " dos not exist\n";
        assert(0);
    }

    if (_gfxlab_texture_dir.empty())
        _gfxlab_texture_dir = _gfxlab_root + "/textures/";
    if (!ValidFolder(_gfxlab_texture_dir)) {
        std::cerr << "GFXLAB_TEXTURE_FOLDER " << _gfxlab_texture_dir << " dos not exist\n";
        assert(0);
    }
}

WindowPtr SceneParser::Parse(const char* file)
{
    SetResourceLocations();

    std::ifstream input(_gfxlab_config_dir+"/"+file);
    if (!input.is_open()) {
        std::cout << "failed to open " << file << std::endl;
        std::exit(-1);
    }
  
    input >> _j;
    auto window = ParseWindow();
    _renderer = ParseRenderer();
    auto scene = ParseScene();
    if (scene != nullptr)
        _renderer->SetScene(scene);
    ParseStateCallbacks();
    ParseRenderPasses();
    window->SetRenderer(_renderer);
    return window;
}

WindowPtr SceneParser::ParseWindow()
{
    std::string title;
    _width = Window::WIDTH;
    _height = Window::HEIGHT;
    title = Window::TITLE;

    LOGINFO("Parsing attribute 'Window'...\n");

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

    return std::shared_ptr<Window>(Window::Create(title.c_str(), _width, _height));
}

ScenePtr SceneParser::ParseScene()
{
    ScenePtr scene = std::shared_ptr<Scene>(nullptr);
    if (_j.find("Scene") != _j.end()) {
        if (_j["Scene"].is_object()) {
            LOGINFO("Parsing attribute 'Scene'...\n");
            auto scene_object = _j["Scene"];
            scene.reset(new Scene());
            if (scene_object.find("camera") != scene_object.end()) {
                if (scene_object["camera"].is_object())
                    ParseCamera(scene, scene_object["camera"]);
                else
                    LOGERR("Expects a JSON object for the attribute Scene.camera\n");
            }
            else {
                CameraPtr cam = std::make_shared<Camera>(0, 0, _width, _height);
                cam->LookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
                scene->SetCamera(cam);
            }

            if (scene_object.find("geometries") != scene_object.end()) {
                if (scene_object["geometries"].is_array())
                    ParseGeometries(scene, scene_object["geometries"]);
                else
                    LOGERR("Expects a JSON array for the attribute Scene.geometries!\n");
            }
            else {
                LOGINFO("No geometries are added to the scene!\n")
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
    if (geom_settings.size() == 0)
        LOGINFO("No geometries are added to the scene!\n");
    
    std::string attib_full_name, source, id, tex;
    int geom_id = 0;
    for (auto& geom : geom_settings) {
        GeometryPtr mesh = std::shared_ptr<Geometry>(new Mesh);
        attib_full_name = "Scene.Geometries[" + std::to_string(geom_id) + "].";
        ProcessStringAttrib(geom, "name", attib_full_name + "name", true, id);
        mesh->SetName(id);

        source = _gfxlab_model_dir + "/" + id;
        ResourceManager::GetInstance()->LoadMesh(source, std::static_pointer_cast<Mesh>(mesh));

        _geometries[id] = mesh;
        scene->AddGeometry(mesh);

        ProcessStringAttrib(geom, "texture", attib_full_name + "texture", false, tex);
        if (!tex.empty()) {
            source = _gfxlab_texture_dir + "/" + tex;
            GLuint tex_id = ResourceManager::GetInstance()->LoadTexture("2D", source);
            mesh->SetTexture(GL_TEXTURE_2D, tex_id);
        }
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
    std::string renderer_name = "default";
    if (_j.find("Renderer") == _j.end()) {
        LOGINFO("Use default renderer.\n");
    }
    else {
        LOGINFO("Parsing attribute 'Renderer'...\n");
        ProcessStringAttrib(_j, "Renderer", "Renderer", true, renderer_name);
    }
    
    return RendererFactory::MakeRenderer(renderer_name);
}

void SceneParser::ParseStateCallbacks()
{
    if (_j.find("SetStateCallbacks") != _j.end()) {
        LOGINFO("Parsing attribute 'SetStateCallbacks'...\n");
        
        auto callback_settings = _j["SetStateCallbacks"];
        std::string library;
        ProcessStringAttrib(callback_settings, "library", "SetStateCallbacks.library", true, library);
        library = _gfxlab_bin_dir + "/" + library;

        std::string global_state_cb;
        std::string per_frame_cb;
        std::string per_program_cb;
        std::string per_geom_cb;

        ProcessStringAttrib(callback_settings, "global_state_cbs", "SetStateCallbacks.global_state_cbs", false, global_state_cb);
        if (global_state_cb.empty())
            global_state_cb = "SetGlobalStates";
        ProcessStringAttrib(callback_settings, "per_frame_cbs", "SetStateCallbacks.per_frame_cbs", false, per_frame_cb);
        if (per_frame_cb.empty())
            per_frame_cb = "SetPerFrameStates";
        ProcessStringAttrib(callback_settings, "per_program_cbs", "SetStateCallbacks.per_program_cbs", false, per_program_cb);
        if (per_program_cb.empty())
            per_program_cb = "SetPerProgramStates";
        ProcessStringAttrib(callback_settings, "per_geom_cbs", "SetStateCallbacks.per_geom_cbs", false, per_geom_cb);
        if (per_geom_cb.empty())
            per_geom_cb = "SetPerGeometryStates";

        auto lib = LoadLibrary(library.c_str());
        if (lib == nullptr) {
            LOGERR("Failed to load library %s: %d\n", library.c_str(), GetLastError());
        }
        else {            
            auto state_cb = GetProcAddress(lib, global_state_cb.c_str());
            if (state_cb != nullptr)
                _renderer->SetGlobalSetStateCallback(reinterpret_cast<void(*)(const ScenePtr&, RenderStates&)>(state_cb));

            
            auto frame_cb = GetProcAddress(lib, per_frame_cb.c_str());
            if (frame_cb != nullptr)
                _renderer->SetFrameSetStateCallback(reinterpret_cast<void(*)(const ScenePtr&, RenderStates&)>(frame_cb));
            

            auto program_cb = GetProcAddress(lib, per_program_cb.c_str());
            if (program_cb != nullptr)
                _renderer->SetProgramSetStateCallback(reinterpret_cast<void(*)(const ScenePtr&, GLuint, RenderStates&, ProgramRenderStates&)>(program_cb));

            auto geom_cb = GetProcAddress(lib, per_geom_cb.c_str());
            if (geom_cb != nullptr)
                _renderer->SetGeometrySetStateCallback(reinterpret_cast<void(*)(const GeometryPtr&, ProgramRenderStates&)>(geom_cb));
        }
    }
    else {
        LOGINFO("No DLL found for setting rendering states\n");
    }
}

void SceneParser::ParseRenderPasses()
{
    if (_j.find("RenderPasses") != _j.end()) {
        LOGINFO("Parsing attribute 'RenderPasses'...\n");

        if (_j["RenderPasses"].is_array()) {
            auto render_passes = _j["RenderPasses"];
            int rp_counter = 0;
            for (auto& rp : render_passes)
                ParseSingleRenderPass(rp, rp_counter++);
        }
        else {
            LOGERR("Expects a JSON array for the attribute 'RenderPasses'\n");
        }
    }
    else {
        LOGERR("No renderpass is found!\n");
    }
}

void SceneParser::ParseSingleRenderPass(const json& rp, int rp_counter)
{
    std::string attrib_full_name = "RenderPasses[" + std::to_string(rp_counter) + "]";

   
    if (rp.is_object()) {
        RenderPassPtr render_pass = std::make_unique<RenderPass>(_renderer);
        
        GLuint prog_id = ParseProgramInRenderPass(rp, attrib_full_name);

        std::vector<GeometryPtr> geometries;
        ParseGeometriesInRenderPass(rp, attrib_full_name, geometries);
        if (geometries.empty())
            render_pass->SetProgram(prog_id);
        else
            render_pass->SetProgramForGeometries(prog_id, geometries);

        if (rp.find("fbo") != rp.end()) {
            std::vector<std::pair<GLuint, GLenum>> color_attachments;
            std::pair<GLuint, GLenum>              depth_attachment;
            std::pair<GLuint, GLenum>              stencil_attachment;
            std::pair<GLuint, GLenum>              ds_attachment;

            ParseFBOAttachmentsInRenderPass(rp["fbo"], color_attachments, depth_attachment, stencil_attachment, ds_attachment, attrib_full_name);
            bool success = render_pass->CreateFBO(color_attachments, depth_attachment, stencil_attachment, ds_attachment);
            if (!success) {
                LOGERR("Failed to create FBO for %s\n", attrib_full_name.c_str());
            }
            RecordFBOInfoForRenderPass(rp_counter, color_attachments, depth_attachment, stencil_attachment, ds_attachment);
        }

        if (rp.find("textures") != rp.end()) {
            std::vector<GLuint> textures;
            ParseInputTextures(rp, rp_counter, textures);
            render_pass->SetInputTextures(textures);
        }

        if (rp.find("show_image") != rp.end()) {
            std::string tex_name;
            ProcessStringAttrib(rp, "show_image", attrib_full_name + ".show_image", true, tex_name);
            GLuint tex_id = GetTextureObj(tex_name);
            assert(tex_id != 0);
            render_pass->SetDisplayImage(tex_id);
        }
            
        _renderer->AddRenderPass(std::move(render_pass));
    }
    else {
        LOGERR("Expects a JSON object for the attribute RenderPasses[%d]\n", rp_counter);
    }
}

GLuint SceneParser::ParseProgramInRenderPass(const json& rp, const std::string& attrib_full_name)
{
    GLuint prog_id;
    if (rp.find("program") != rp.end()) {
        std::string prog_name, shaders;

        ProcessStringAttrib(rp["program"], "name", attrib_full_name + "name", true, prog_name);
        ProcessStringAttrib(rp["program"], "shaders", attrib_full_name + "shaders", true, shaders);

        if (_programs.find(prog_name) == _programs.end()) {
            std::vector<std::string> shader_files;
            CollectShaderFiles(shaders, shader_files);
            prog_id = ResourceManager::GetInstance()->CreateProgram(shader_files);
            assert(prog_id != 0);
            _renderer->AddShaderProgram(prog_name, prog_id);
            _programs[prog_name] = prog_id;
        }
        else {
            prog_id = _programs[prog_name];
        }
        return prog_id;
    }
    else {
        LOGERR("Attribute 'program' not found in %s!\n", attrib_full_name.c_str());
    }
}

void SceneParser::ParseGeometriesInRenderPass(const json& rp, const std::string attrib_full_name, std::vector<GeometryPtr>& geometries)
{
    std::vector<std::string> geom_names;
    ProcessStringArrayAttrib(rp, "geometries", attrib_full_name + ".geometries", false, geom_names);
    for (auto& name : geom_names)
        geometries.push_back(_geometries[name]);
}

void SceneParser::RecordFBOInfoForRenderPass(GLuint fbo,
    std::vector<std::pair<GLuint, GLenum>>& color_attachments,
    std::pair<GLuint, GLenum>& depth_attachment,
    std::pair<GLuint, GLenum>& stencil_attachment,
    std::pair<GLuint, GLenum>& ds_attachment)
{
    FBOAttachments info;
    memset(&info, 0, sizeof(FBOAttachments));

    for (auto& color : color_attachments)
        info.color.push_back(color.first);
    
    info.depth = depth_attachment.first;
    info.stencil = stencil_attachment.first;
    info.depth_stencil = ds_attachment.first;

    _fboAttachments[fbo] = info;
}

void SceneParser::ParseFBOAttachmentsInRenderPass(const json& fbo,
                                      std::vector<std::pair<GLuint, GLenum>>& color_attachments,
                                      std::pair<GLuint, GLenum>& depth_attachment,
                                      std::pair<GLuint, GLenum>& stencil_attachment,
                                      std::pair<GLuint, GLenum>& ds_attachment,
                                      const std::string& rp_name)
{
    GLint max_color_attachments;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_color_attachments);

    std::string full_attrib_name = rp_name + ".fbo.";
    std::string color_attachment_name;
    FBOInfo fbo_info;
    for (GLint i = 0; i < max_color_attachments; i++) {
        color_attachment_name = "color" + std::to_string(i);
        if (fbo.find(color_attachment_name) != fbo.end()) {
            fbo_info = CreateFBOAttachment(fbo[color_attachment_name], "color", full_attrib_name + color_attachment_name);
            color_attachments.push_back(std::make_pair(fbo_info.first, fbo_info.second));
        }
        else {
            break;
        }
    }

    if (fbo.find("depth") != fbo.end()) {
        depth_attachment = CreateFBOAttachment(fbo["depth"], "depth", full_attrib_name + "depth");
    }
   
    if (fbo.find("depth_stencil") != fbo.end()) {
        ds_attachment = CreateFBOAttachment(fbo["depth_stencil"], "depth_stencil", full_attrib_name + "depth_stencil");
    }

    if (fbo.find("stencil") != fbo.end()) {
        stencil_attachment = CreateFBOAttachment(fbo["stencil"], "stencil", full_attrib_name + "stencil");
    }
}

FBOInfo SceneParser::CreateFBOAttachment(const json& attachment, const std::string& type, const std::string& full_attrib_name)
{
    const static int TEXTURE = 0;
    const static int RENDERBUFFER = 1;

    GLsizei width, height;
    width = _width;
    height = _height;

    GLenum attachment_type = GL_TEXTURE_2D;
    std::string type_str;
    ProcessStringAttrib(attachment, "type", "type", false, type_str);
    if (!type_str.empty()) {
        if (type_str == "render buffer")
            attachment_type = GL_RENDERBUFFER;
        else if (type_str == "texture")
            attachment_type = GL_TEXTURE_2D;
        else
            LOGERR("%s: unsupported FBO attachment type: %s. Must be either 'render buffer' or 'texture'\n",
                full_attrib_name.c_str(),
                type_str.c_str());
    }

    std::vector<float> dimension;
    ProcessNumberArrayAttrib(attachment, "dimension", "dimension", false, dimension);
    if (!dimension.empty()) {
        if (dimension.size() == 2) {
            width = (GLsizei)dimension[0];
            height = (GLsizei)dimension[1];
        }
        else {
            LOGERR("The 'dimension' attribute of a FBO attachment must be an array of size 2\n")
        }
    }

    GLuint id;
    if (attachment_type == GL_TEXTURE_2D) {
        id = ResourceManager::GetInstance()->CreateTexture(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE);
    }
    else {
        GLenum format;
        if (type == "depth")
            format = GL_DEPTH_COMPONENT;
        else if (type == "stencil")
            format = GL_STENCIL_INDEX;
        else
            format = GL_DEPTH24_STENCIL8;
        id = ResourceManager::GetInstance()->CreateRenderBuffer(GL_RENDERBUFFER, format, width, height);
    }

    return std::make_pair(id, attachment_type);
}


void SceneParser::ParseInputTextures(const json& textures, int rp, std::vector<GLuint>& tex_objs)
{
    std::vector<std::string> str_textures;
    std::string full_name = "RenderPasses[" + std::to_string(rp) + "].textures";
    ProcessStringArrayAttrib(textures, "textures", full_name, false, str_textures);

    for (auto& tex_name : str_textures) {
        GLuint tex_id = GetTextureObj(tex_name);
        if (tex_id != 0) {
            tex_objs.push_back(tex_id);
        }
        else {
            LOGERR("%s does not exist\n", tex_name.c_str());
        }
    }
}

GLuint SceneParser::GetTextureObj(const std::string& tex_name)
{
    std::regex pattern("fbo[0-9]+\\.(color[0-9]+|depth|stencil|ds)");
    GLuint tex_id = 0;

    if (std::regex_match(tex_name, pattern)) {
        size_t dot_pos = tex_name.find_first_of('.');
        std::string fbo_id_str = tex_name.substr(3, dot_pos + 1);
        GLuint fbo_id = std::atoi(fbo_id_str.c_str());
        if (_fboAttachments.find(fbo_id) != _fboAttachments.end()) {
            FBOAttachments fbo_info = _fboAttachments[fbo_id];

            if (tex_name.find("color") != std::string::npos) {
                std::string color_idx_str = tex_name.substr(dot_pos + strlen("color") + 1);
                int color_idx = std::atoi(color_idx_str.c_str());
                assert(color_idx < fbo_info.color.size() && fbo_info.color[color_idx] > 0);
                tex_id = fbo_info.color[color_idx];
            }
            else if (tex_name.find("depth") != std::string::npos) {
                tex_id = fbo_info.depth;
            }
            else if (tex_name.find("stencil") != std::string::npos) {
                tex_id = fbo_info.stencil;
            }
            else if (tex_name.find("ds") != std::string::npos) {
                tex_id = fbo_info.depth_stencil;
            }
        }
    }
    else {
        std::string  source = _gfxlab_texture_dir + "/" + tex_name;
        tex_id = ResourceManager::GetInstance()->LoadTexture("2D", source);
    }

    return  tex_id;
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

void SceneParser::CollectShaderFiles(const std::string& shaders, std::vector<std::string>& shader_files)
{
    size_t prev = 0, next;
    while ((next = shaders.find_first_of(";", prev)) != std::string::npos) {
        shader_files.push_back(_gfxlab_shader_dir + "/" + shaders.substr(prev, next - prev));
        next++;
        while (std::isspace(shaders[next]))
            next++;
        prev = next;
    }
    shader_files.push_back(_gfxlab_shader_dir + "/" + shaders.substr(prev));

}


