#pragma once

#include "renderstatecallbacks.h"

class RenderPass {
public:
    RenderPass(RendererPtr renderer) : _renderer(renderer) {}

    void      SetProgramForGeometries(GLuint prog_id, const std::vector<GeometryPtr>& geoms)
    {
        _geometriesByProgram[prog_id] = geoms;
    }

    void      Render();

private:
    RendererPtr                                            _renderer;
    std::unordered_map<GLuint, std::vector<GeometryPtr>>   _geometriesByProgram;
    std::unordered_map<GLuint, SetPerProgramStateCallback> _programCallbacks;
};