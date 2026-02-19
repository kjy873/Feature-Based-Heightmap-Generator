#pragma once

#include <glew.h>
#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>

#include <map>
#include <unordered_map>

#include <iostream>

#include <glfw3.h>

#include "Features.h"

struct DebugTextures {
	GLuint Tex0;
	GLuint Tex1;
	int ResU, ResV;
	int unit0 = 8;
	int unit1 = 9;
};

struct GradientRGBA {
	float nx;
	float ny;
	float norm;
	float pad = 0.0f;
};

struct PingPongTexture {
	GLuint Texture[2] = { 0,0 };
	int ping = 0;

	GLuint& GetReadTexture() { return Texture[ping]; }
	GLuint& GetWriteTexture() { return Texture[1 - ping]; }
	void Swap() { ping = 1 - ping; }
};

struct GPUTextures {
	PingPongTexture Elevation;  // level 1부터는 오차 해를 계산하는 텍스처로 사용
	PingPongTexture Gradient;  // level 1부터는 계산된 F_G를 저장하는 텍스처로 사용

	GLuint Residual_Fine;
	GLuint Residual_Coarse;

	PingPongTexture Correction;

	GLuint DebugTexture = 0;

	GLuint SSBO = 0;
};



struct BufferData {
	GLuint VBO_Position;
	GLuint VBO_Height;
	GLuint VBO_Color;
	GLuint VBO_Normal;
	GLuint VAO;
	GLuint EBO = 0;

	int VertexCount;
	int IndexCount;
};

// 다른 서식마다 다른 VAO로 관리, VBO 이름은 바뀔 수 있음
class BufferManager
{
private:
	std::unordered_map<unsigned int, BufferData> BufferMap;

	std::vector<GPUTextures> Textures;
	
	unsigned int BufferID = 0;

	DebugTextures DebugTexture;

public:

	BufferManager() {}
	~BufferManager() {}

	void InitVertexArrayObejct(BufferData &BufferData, bool UseEBO);
	void BindVertexBufferObject(BufferData& BufferData, const glm::vec3* Position, int PositionSize, const glm::vec3* Color, int ColorSize, const glm::vec3* Normal, int NormalSize);
	void BindElementBufferObject(BufferData& BufferData, const int* Index, int IndexSize);
	void UploadHeight(BufferData& BufferData, const float* HeightMap, int Size);

	unsigned int CreateMeshID() { return BufferID++; };

	BufferData& CreateBufferData(unsigned int ID, bool UseEBO);
	void BindVertexBufferObjectByID(unsigned int ID, const glm::vec3* Position, int PositionSize, const glm::vec3* Color, int ColorSize, const glm::vec3* Normal, int NormalSize);
	void BindElementBufferObjectByID(unsigned int ID, const int* Index, int IndexSize);

	void UploadHeightByID(unsigned int ID, const float* HeightMap, int Size);

	void AddTextureSet() { Textures.emplace_back(); }

	void UploadElevationTexture(int ResU, int ResV,
		const float* ElevationMap, const std::vector<glm::vec2>& NoiseMap, const uint8_t* ConstraintMaskMap,
		const int Index);
	void UploadGradientTexture(int ResU, int ResV, const std::vector<glm::vec3>& GradientMap, const int Index);
	void UploadNoiseTexture(int ResU, int ResV, const std::vector<glm::vec2>& NoiseMap, const int Index);
	void UploadConstraintMaskTexture(int ResU, int ResV, const uint8_t* ConstraintMaskMap, const int Index);
	void UploadEmptyTextures(int ResU, int ResV);
	void BindElevationTextureDiffusion(const int Index);
	void BindElevationTextureResidual(const int Index);
	void BindGradientTexture(const int Index);
	void BindNoiseTexture(const int Index);
	void BindConstraintMaskTexture(const int Index);
	void BindTexturesDefault();

	

	void AllocateRasidualTexture(const int ResU, const int ResV, const int Index);
	void BindResidualTextureRead(const int Index);
	void BindResidualTextureWrite(const int Index);

	void AllocateCoarseTextures(const int ResU, const int ResV, const int Index);
	void BindCoarseTextureWriteInResidualPass(const int Index);
	void BindCoarseTextureInCoarsePass(const int Index);
	void BindFineGradeintInResidualPass(const int Index);
	void BindCoarseTextureInCorrectionPass(const int Index);


	void UploadDbgTexture(int ResU, int ResV, const int Index);
	void BindDbgTexture(const int Index);

	void SwapElevation(const int Index);
	void SwapGradient(const int Index);
	void SwapNoise(const int Index);

	void ResetGradientPingPong(const int Index);

	void BindGradientReadOnly(const int Index);

	const GLuint& GetElevationTextureWrite(const int Index);


	void UnbindAllTextures();
	void UnbindElevationTexture();


	std::vector<float> ReadbackElevationTexture(int ResU, int ResV, const int Index);
	std::vector<glm::vec3> ReadbackElevationTextureVec3(int ResU, int ResV, const int Index);
	std::vector<glm::vec3> ReadbackGradientTexture(int ResU, int ResV, const int Index);
	std::vector<glm::vec2> ReadbackNoiseTexture(int ResU, int ResV, const int Index);
	std::vector<glm::vec4> ReadbackDbgTexture(int ResU, int ResV, const int Index);
	std::vector<float> ReadbackResidualTexture(int ResU, int ResV, const int Index);

	void CreateSSBO(const int Index);
	void BindSSBO(const int Index);

	void UploadDebugPixel2(const GLuint ProgramID, const glm::ivec2& p0, const glm::ivec2& p1);
	void AskDebugPixel2(const GLuint ProgramID, const glm::ivec2& p0, const glm::ivec2& p1, const int Index);

	void ReadPrintSSBO(const int Index);

	void AllocateTexture2D(GLuint& TextureID, int ResU, int ResV, GLenum InternalFormat, GLenum Format, GLenum Type, GLenum filter = GL_NEAREST, GLenum wrap = GL_CLAMP_TO_EDGE);
	void CreateDebugTextures(int ResU, int ResV);
	void UploadDebugTextures(const std::vector<glm::vec4>& PackedRGBA, const std::vector<glm::vec4>& PackedRGRC);
	void BindDebugTextures(GLuint ShaderProgramID);

	const GLuint& GetVAOByID(unsigned int ID) const{
		auto it = BufferMap.find(ID);
		if (it == BufferMap.end()) {
			std::cerr << "Error: Buffer ID " << ID << " not found." << std::endl;
			throw std::runtime_error("Buffer ID not found");
		}
		return it->second.VAO;
	}
};
