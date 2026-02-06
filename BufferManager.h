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

struct GradientRGBA {
	float nx;
	float ny;
	float norm;
	float pad = 0.0f;
};

struct PingPongTexture {
	GLuint Texture[2] = { 0,0 };
	int ping = 0;

	GLuint GetReadTexture() const { return Texture[ping]; }
	GLuint GetWriteTexture() const { return Texture[1 - ping]; }
	void Swap() { ping = 1 - ping; }
};

struct GPUTextures {
	PingPongTexture Elevation;
	PingPongTexture Gradient;
	PingPongTexture Noise;
	PingPongTexture Multigrid;
	GLuint ConstraintMask;

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

	GPUTextures Textures;
	
	unsigned int BufferID = 0;

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

	void UploadElevationTexture(int ResU, int ResV, const float* ElevationMap);
	void UploadGradientTexture(int ResU, int ResV, const std::vector<glm::vec3>& GradientMap);
	void UploadNoiseTexture(int ResU, int ResV, const std::vector<glm::vec2>& NoiseMap);
	void UploadConstraintMaskTexture(int ResU, int ResV, const uint8_t* ConstraintMaskMap);
	void UploadEmptyTextures(int ResU, int ResV);
	void BindElevationTexture();
	void BindGradientTexture();
	void BindNoiseTexture();
	void BindConstraintMaskTexture();
	void BindTexturesDefault();

	void UploadDebugTexture(int ResU, int ResV);
	void BindDebugTexture();

	void SwapElevation() { Textures.Elevation.Swap(); }
	void SwapGradient() { Textures.Gradient.Swap(); }
	void SwapNoise() { Textures.Noise.Swap(); }

	void ResetGradientPingPong() { Textures.Gradient.ping = 0; }
	void BindGradientReadOnly();

	const GLuint& GetElevationTextureWrite() { return Textures.Elevation.GetWriteTexture(); }

	void UnbindAllTextures();
	void UnbindElevationTexture();


	std::vector<float> ReadbackElevationTexture(int ResU, int ResV);
	std::vector<glm::vec3> ReadbackElevationTextureVec3(int ResU, int ResV);
	std::vector<glm::vec3> ReadbackGradientTexture(int ResU, int ResV);
	std::vector<glm::vec4> ReadbackDebugTexture(int ResU, int ResV);

	void CreateSSBO();
	void BindSSBO();

	void UploadDebugPixel2(const GLuint ProgramID, const glm::ivec2& p0, const glm::ivec2& p1);
	void AskDebugPixel2(const GLuint ProgramID, const glm::ivec2& p0, const glm::ivec2& p1);

	void ReadPrintSSBO();


	const GLuint& GetVAOByID(unsigned int ID) const{
		auto it = BufferMap.find(ID);
		if (it == BufferMap.end()) {
			std::cerr << "Error: Buffer ID " << ID << " not found." << std::endl;
			throw std::runtime_error("Buffer ID not found");
		}
		return it->second.VAO;
	}
};
