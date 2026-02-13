#pragma once

#include "BufferManager.h"
#include "ShaderManager.h"
#include "Mesh.h"

// 이 클래스로 만든 객체는 BufferManager, ShaderManager보다 늦게 생성되고 소멸되어야 함

enum class DebugMode : int {
	None = 0,
	Elevation = 1,
	NormXY = 2,
	NormZ = 3,
	Amplitude = 4,
	Roughness = 5
};

struct RenderItem {
	unsigned int MeshID;
	glm::mat4 ModelTransform;
};

class RenderManager
{

private:

	const BufferManager& BufferMgr;

	const ShaderManager& ShaderMgr;

	GLuint ViewLocation;
	GLuint ProjLocation;
	GLuint ModelLocation;
	GLuint LightLocation;

	GLuint DebugModeLocation;
	GLuint DebugOverlayAlphaLocation;
	GLuint ResLocation;

	GLuint HighlightWeightLocation;



public:
	RenderManager(const ShaderManager& ShaderMgr, const BufferManager& BufferMgr) : ShaderMgr(ShaderMgr), BufferMgr(BufferMgr) {};

	void Init(const char* view,
		const char* proj,
		const char* model,
		const char* light,
		const char* HighlightWeight,
		const char* debugmode,
		const char* debugoverlayalpha,
		const char* Res);

	void BeginFrame(const glm::mat4& View, const glm::mat4& Proj, const glm::vec3& LightPos, const DebugMode Mode, const float DebugAlpha, const int ResU, const int ResV);

	void Draw(const Mesh& Mesh);

	void DrawWireframe(const Mesh& Mesh);

	void UploadHighlightWeight(float Weight) { glUniform1f(HighlightWeightLocation, Weight); }

};