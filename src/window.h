#pragma once
#include "common.h"

class Renderer;



class Window {
public:
	~Window();

	void           SetRenderer(RendererPtr renderer) { _renderer = renderer; }
	void           Display();
	static Window* Create(const char* title, int width, int height);

	static const int         WIDTH   = 800;
	static const int         HEIGHT  = 600;
	static const char* const TITLE;
private: 
	Window(const char* title, int width, int height);
	static void       cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void       mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void       scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void       window_size_callback(GLFWwindow* window, int width, int height);

	static Window*           _window;
	static CameraPtr         _activeCamera;
	int                      _width;
	int                      _height;
	std::string              _title;

	GLFWwindow*              _glfwWindow;
	RendererPtr              _renderer;
};