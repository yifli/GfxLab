#include <renderstatecallbacks.h>
#include <scene.h>
#include <geometry.h>
#include <light.h>
#include <camera.h>

#include <glm/gtc/type_ptr.hpp>


void SetGlobalStates(const ScenePtr& scene, RenderStates& rs)
{
	// find uniform locations
	GLuint prog = rs.programs["lighting"];
	rs.program_states[prog].uniform_locations["light_pos"] = glGetUniformLocation(prog, "light_pos");

	rs.program_states[prog].uniform_locations["model"] = glGetUniformLocation(prog, "model");
	rs.program_states[prog].uniform_locations["view"] = glGetUniformLocation(prog, "view");
	rs.program_states[prog].uniform_locations["projection"] = glGetUniformLocation(prog, "projection");

    prog = rs.programs["display_normal"];
    rs.program_states[prog].uniform_locations["model"] = glGetUniformLocation(prog, "model");
    rs.program_states[prog].uniform_locations["view"] = glGetUniformLocation(prog, "view");
    rs.program_states[prog].uniform_locations["projection"] = glGetUniformLocation(prog, "projection");
}


void SetPerProgramStates(const ScenePtr& scene, GLuint prog, RenderStates& rs, ProgramRenderStates& prog_rs)
{
    if (rs.reverse_program_lookup[prog] == "lighting") {
        GLfloat pos[3] = { 0, 0, 10 };
        glUniform3fv(rs.program_states[prog].uniform_locations["light_pos"], 1, pos);
    }
	glUniformMatrix4fv(rs.program_states[prog].uniform_locations["view"], 1, GL_FALSE, glm::value_ptr(scene->GetCamera()->GetViewMatrix()));
	glUniformMatrix4fv(rs.program_states[prog].uniform_locations["projection"], 1, GL_FALSE, glm::value_ptr(scene->GetCamera()->GetProjectionMatrix()));
}

void SetPerGeometryStates(const GeometryPtr& geom, ProgramRenderStates& prog_rs)
{
	glUniformMatrix4fv((prog_rs.uniform_locations)["model"], 1, GL_FALSE, glm::value_ptr(geom->GetTransformation()));
}