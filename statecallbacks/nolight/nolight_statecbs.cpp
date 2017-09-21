#include <renderstatecallbacks.h>
#include <scene.h>
#include <geometry.h>
#include <light.h>
#include <camera.h>

#include <glm/gtc/type_ptr.hpp>


void SetGlobalStates(const ScenePtr& scene, RenderStates& rs)
{
    // find uniform locations
    GLuint passthrough_prog = rs.programs["passthrough"];
    rs.program_states[passthrough_prog].uniform_locations["model"] = glGetUniformLocation(passthrough_prog, "model");
    rs.program_states[passthrough_prog].uniform_locations["view"] = glGetUniformLocation(passthrough_prog, "view");
    rs.program_states[passthrough_prog].uniform_locations["projection"] = glGetUniformLocation(passthrough_prog, "projection");
}


void SetPerProgramStates(const ScenePtr& scene, GLuint prog, RenderStates& rs, ProgramRenderStates& prog_rs)
{
    glUniformMatrix4fv(rs.program_states[prog].uniform_locations["view"], 1, GL_FALSE, glm::value_ptr(scene->GetCamera()->GetViewMatrix()));
    glUniformMatrix4fv(rs.program_states[prog].uniform_locations["projection"], 1, GL_FALSE, glm::value_ptr(scene->GetCamera()->GetProjectionMatrix()));
}

void SetPerGeometryStates(const GeometryPtr& geom, ProgramRenderStates& prog_rs)
{
    glUniformMatrix4fv((prog_rs.uniform_locations)["model"], 1, GL_FALSE, glm::value_ptr(geom->GetTransformation()));
}