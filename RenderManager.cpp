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

	if (mesh.GetPosition().size() == 0) return;

	glUseProgram(ShaderMgr.GetShaderProgramID());

	glBindVertexArray(BufferMgr.GetVAOByID(mesh.GetMeshID()));
	glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(mesh.GetTransformMatrix()));
	//glDrawArrays(mesh.GetDrawMode(), 0, mesh.GetPosition().size());

	//std::cout << "Start Draw" << std::endl;

	const auto& HasIndex = mesh.GetIndex();
	//if (!HasIndex.empty()) std::cout << "Drawing with Indices: " << HasIndex.size() << " indices." << std::endl;
	//else std::cout << "Drawing without Indices: " << mesh.GetPosition().size() << " vertices." << std::endl;
	if (!HasIndex.empty()) glDrawElements(mesh.GetDrawMode(), HasIndex.size(), GL_UNSIGNED_INT, 0);
	else glDrawArrays(mesh.GetDrawMode(), 0, mesh.GetPosition().size());

	//std::cout << "End Draw" << std::endl;

}

void RenderManager::DrawWireframe(const Mesh& mesh) {

	glUseProgram(ShaderMgr.GetShaderProgramID());

	glBindVertexArray(BufferMgr.GetVAOByID(mesh.GetMeshID()));
	glUniformMatrix4fv(ModelLocation, 1, GL_FALSE, glm::value_ptr(mesh.GetTransformMatrix()));
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glDrawArrays(mesh.GetDrawMode(), 0, mesh.GetPosition().size());

	const auto& HasIndex = mesh.GetIndex();
	//if (!HasIndex.empty()) std::cout << "Drawing with Indices: " << HasIndex.size() << " indices." << std::endl;
	//else std::cout << "Drawing without Indices: " << mesh.GetPosition().size() << " vertices." << std::endl;
	if (!HasIndex.empty()) glDrawElements(mesh.GetDrawMode(), HasIndex.size(), GL_UNSIGNED_INT, 0);
	else glDrawArrays(mesh.GetDrawMode(), 0, mesh.GetPosition().size());

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

