#include <Windows.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <iostream>

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        glewExperimental = GL_TRUE;
        GLenum status = glewInit();
        if (GLEW_OK != status) {
            std::cout << "glewInit failed: " << glewGetErrorString(status) << std::endl;
            std::exit(-1);
        }
        break;
    }

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:
        // Perform any necessary cleanup.
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}