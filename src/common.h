#pragma once


#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <unordered_set>
#include <iostream>
#include <mutex>
#include <string>
#include <cassert>


class Window;
class Scene;
class Renderer;
class Camera;
class Light;
class Geometry;
class Mesh;
class ResourceManager;

using WindowPtr   = std::unique_ptr<Window>;
using ScenePtr    = std::shared_ptr<Scene>;
using RendererPtr = std::shared_ptr<Renderer>;
using CameraPtr   = std::shared_ptr<Camera>;
using LightPtr    = std::shared_ptr<Light>;
using GeometryPtr = std::shared_ptr<Geometry>;
using MeshPtr     = std::shared_ptr<Mesh>;
using ResourcePtr = std::unique_ptr<ResourceManager>;
