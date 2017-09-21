#include "renderpass.h"
#include "renderer.h"
#include "geometry.h"

void RenderPass::Render()
{
    for (auto& kv : _geometriesByProgram) {
        GLuint prog_id = kv.first;
        auto& geometries = kv.second;

        glUseProgram(prog_id);
        auto& prog_cb = _renderer->_perProgramCallback;
        prog_cb(_renderer->_scene, prog_id, _renderer->_renderStates, _renderer->_renderStates.program_states[prog_id]);

        for (auto& g : geometries) {
            _renderer->_perGeometryCallback(g, _renderer->_renderStates.program_states[prog_id]);
            g->Render();
        }

        glUseProgram(0);
    }
}