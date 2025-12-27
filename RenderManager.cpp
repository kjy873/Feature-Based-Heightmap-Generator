#include "RenderManager.h"


void RenderManager::Init(const char* view, const char* proj, const char* model, const char* light) {

	ViewLocation = glGetUniformLocation(ShaderMgr.GetShaderProgramID(), view);
	ProjLocation = glGetUniformLocation(ShaderMgr.GetShaderProgramID(), proj);
	ModelLocation = glGetUniformLocation(ShaderMgr.GetShaderProgramID(), model);
	LightLocation = glGetUniformLocation(ShaderMgr.GetShaderProgramID(), light);

}

void RenderManager::BeginFrame(const glm::mat4& View, const glm::mat4& Proj, const glm::vec3 &LightPos) {

	glUseProgram(ShaderMgr.GetShaderProgramID());

	//glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(ViewLocation, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(ProjLocation, 1, GL_FALSE, &Proj[0][0]);
}

void RenderManager::Draw(const Mesh& mesh) {

	glUseProgram(ShaderMgr.GetShaderProgramID());

	glBindVertexArray(BufferMgr.GetVAOByID(mesh.GetMeshID()));
	glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(mesh.GetTransformMatrix()));
	glDrawArrays(mesh.GetDrawMode(), 0, mesh.GetVertexCount());

}