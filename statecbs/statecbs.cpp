#include <renderstatecallbacks.h>
#include <scene.h>
#include <geometry.h>
#include <light.h>
#include <camera.h>

#include <glm/gtc/type_ptr.hpp>

void InitLights(GLuint light_program, const ScenePtr& scene)
{
	int n_pointlights, n_dirlights;
	n_pointlights = n_dirlights = 0;

	glUseProgram(light_program);
	for (auto& light : scene->GetLights()) {
		if (light->GetType() == Light::POINT) {
			std::string element = "pointLights[" + std::to_string(n_pointlights) + "].";
			glUniform3fv(glGetUniformLocation(light_program, (element + "position").c_str()), 1, glm::value_ptr(light->GetPosition()));
			glUniform3fv(glGetUniformLocation(light_program, (element + "ambient").c_str()), 1, glm::value_ptr(light->GetAmbient()));
			glUniform3fv(glGetUniformLocation(light_program, (element + "diffuse").c_str()), 1, glm::value_ptr(light->GetDiffuse()));
			glUniform3fv(glGetUniformLocation(light_program, (element + "sepcular").c_str()), 1, glm::value_ptr(light->GetSpecular()));
			glUniform1f(glGetUniformLocation(light_program, (element + "linear").c_str()), light->GetLinearAtten());
			glUniform1f(glGetUniformLocation(light_program, (element + "constant").c_str()), light->GetConstantAtten());
			glUniform1f(glGetUniformLocation(light_program, (element + "quadratic").c_str()), light->GetQuadraticAtten());
			n_pointlights++;
		}
		else if (light->GetType() == Light::DIRECTION) {
			std::string element = "dirLights[" + std::to_string(n_pointlights) + "].";
			glUniform3fv(glGetUniformLocation(light_program, (element + "direction").c_str()), 1, glm::value_ptr(light->GetDirection()));
			glUniform3fv(glGetUniformLocation(light_program, (element + "ambient").c_str()), 1, glm::value_ptr(light->GetAmbient()));
			glUniform3fv(glGetUniformLocation(light_program, (element + "diffuse").c_str()), 1, glm::value_ptr(light->GetDiffuse()));
			glUniform3fv(glGetUniformLocation(light_program, (element + "diffuse").c_str()), 1, glm::value_ptr(light->GetSpecular()));
			n_dirlights++;
		}
	}
	glUniform1i(glGetUniformLocation(light_program, "numDirLights"), n_dirlights);
	glUniform1i(glGetUniformLocation(light_program, "numPointLights"), n_pointlights);
	glUseProgram(0);
}

void InitUBOs(RenderStates& rs)
{
	GLuint light_program = rs.programs["lighting"];
	GLuint uniformBlockIndex = glGetUniformBlockIndex(light_program, "Matrices");
	glUniformBlockBinding(light_program, uniformBlockIndex, 0);

	GLuint ubo;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, 2 * sizeof(glm::mat4));
	rs.uniform_buffer_objects.push_back(ubo);
}

void SetGlobalStates(const ScenePtr& scene, RenderStates& rs)
{
	InitLights(rs.programs["lighting"], scene);
	InitUBOs(rs);
}

void SetPerFrameStates(const ScenePtr& scene, const RenderStates& rs)
{
	glBindBuffer(GL_UNIFORM_BUFFER, rs.uniform_buffer_objects[0]);
	auto cam = scene->GetCamera();
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(cam->GetProjectionMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(cam->GetViewMatrix()));
}

void SetPerProgramStates(const ScenePtr& scene, GLuint prog, RenderStates& rs, ProgramRenderStates& prog_rs)
{
	std::string progname = rs.reverse_program_lookup[prog];
	glUseProgram(prog);

	if (progname == "lighting") {
		prog_rs.uniform_locations["objectColor"] =  glGetUniformLocation(prog, "objectColor");
		prog_rs.uniform_locations["model"] = glGetUniformLocation(prog, "model");
		prog_rs.uniform_locations["viewPos"] = glGetUniformLocation(prog, "viewPos");
		prog_rs.uniform_locations["useTexture"] = glGetUniformLocation(prog, "useTexture");

		glUniform3fv(prog_rs.uniform_locations["viewPos"], 1, glm::value_ptr(scene->GetCamera()->GetPosition()));
	}

	glUseProgram(0);
}

void SetPerGeometryStates(const GeometryPtr& geom, ProgramRenderStates& prog_rs)
{
	if (geom->GetName() == "cube") {
		glUniformMatrix4fv((prog_rs.uniform_locations)["model"], 1, GL_FALSE, glm::value_ptr(geom->GetTransformation()));
		glUniform1i((prog_rs.uniform_locations)["useTexture"], geom->UsesTexture());
	}
}