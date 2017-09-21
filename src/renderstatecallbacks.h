#pragma once
#include "common.h"

// data structures for communicating render states between main application and callback DLLs
struct ProgramRenderStates {
    std::unordered_map<std::string, GLuint>              uniform_locations;
};

struct RenderStates {
    std::vector<GLuint>                                  uniform_buffer_objects;
    std::unordered_map<GLuint, ProgramRenderStates>      program_states;
    std::unordered_map<std::string, GLuint>              programs;
    std::unordered_map<GLuint, std::string>              reverse_program_lookup;
};


// callback type declarations used by main application
using SetGlobalStateCallback      = std::function<void(const ScenePtr&, RenderStates&)>;
using SetPerFrameStateCallback    = std::function<void(const ScenePtr&, RenderStates&)>;
using SetPerProgramStateCallback  = std::function<void(const ScenePtr&, GLuint, RenderStates&, ProgramRenderStates&)>;
using SetPerGeometryStateCallback = std::function<void(const GeometryPtr&, ProgramRenderStates&)>;
