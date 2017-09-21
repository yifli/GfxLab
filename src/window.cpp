#include "window.h"
#include "camera.h"
#include "renderer.h"
#include "renderpass.h"

Window*           Window::_window = nullptr;
CameraPtr         Window::_activeCamera = nullptr;
const char* const Window::TITLE = "GfxLab";

Window* Window::Create(const char* title, int width, int height)
{
	if (_window == nullptr)
		_window = new Window(title, width, height);
	return _window;
}

Window::Window(const char* title, int width, int height)
	: _width(width),
	_height(height),
	_title(title),
	_glfwWindow(nullptr),
	_renderer(nullptr)
{
	if (!glfwInit()) {
		std::cout << "glfwInit failed" << std::endl;
		std::exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	_glfwWindow = glfwCreateWindow(_width, _height, _title.c_str(), NULL, NULL);
	if (!_glfwWindow) {
		glfwTerminate();
		std::cout << "failed to create window" << std::endl;
		std::exit(-1);
	}
	glfwMakeContextCurrent(_glfwWindow);
	glfwSetCursorPosCallback(_glfwWindow, &(Window::cursor_position_callback));
	glfwSetScrollCallback(_glfwWindow, &(Window::scroll_callback));
	glfwSetKeyCallback(_glfwWindow, &(Window::key_callback));
	glfwSetMouseButtonCallback(_glfwWindow, &(Window::mouse_button_callback));
	glfwSetWindowSizeCallback(_glfwWindow, &(Window::window_size_callback));
	glfwSetInputMode(_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	GLenum status = glewInit();
	if (GLEW_OK != status) {
		std::cout << "glewInit failed: " << glewGetErrorString(status) << std::endl;
		std::exit(-1);
	}
}

Window::~Window()
{
	if (_glfwWindow)
		glfwDestroyWindow(_glfwWindow);
	glfwTerminate();
}

void Window::Display()
{
	if (_renderer != nullptr)
		_renderer->Initialize();

	while (!glfwWindowShouldClose(_glfwWindow)) {
		if (_renderer != nullptr)
			_renderer->Render();
		glfwPollEvents();
		glfwSwapBuffers(_glfwWindow);
	}
}

void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (_window->_renderer != nullptr) {
		_activeCamera = _window->_renderer->GetCamera();
		if (_activeCamera != nullptr)
			_activeCamera->Rotate(float(xpos), float(ypos));
	}
	
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && _activeCamera != nullptr) {
		if (action == GLFW_PRESS)
			_activeCamera->BeginRotate();
		else if (action == GLFW_RELEASE)
			_activeCamera->StopRotate();
	}
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key) {
	case GLFW_KEY_A:
		_activeCamera->MoveLeft();
		break;
	case GLFW_KEY_D:
		_activeCamera->MoveRight();
		break;
	case GLFW_KEY_W:
		_activeCamera->MoveForward();
		break;
	case GLFW_KEY_S:
		_activeCamera->MoveBackward();
		break;
	default:
		break;
	}

	if (_window->_renderer)
		_window->_renderer->OnKeyPressed(key, scancode, action, mods);

}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (_activeCamera != nullptr)
		_activeCamera->Zoom(float(yoffset));
}

void Window::window_size_callback(GLFWwindow* window, int width, int height)
{
	if (_window->_renderer != nullptr)
		_window->_renderer->Resize(width, height);
}
