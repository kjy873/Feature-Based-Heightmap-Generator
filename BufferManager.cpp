#include "BufferManager.h"

void BufferManager::InitVertexArrayObejct(BufferData &BufferData, bool UseEBO) {
	glGenVertexArrays(1, &BufferData.VAO);
	glBindVertexArray(BufferData.VAO);
	
	glGenBuffers(1, &BufferData.VBO_Position);
	glBindBuffer(GL_ARRAY_BUFFER, BufferData.VBO_Position);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &BufferData.VBO_Color);
	glBindBuffer(GL_ARRAY_BUFFER, BufferData.VBO_Color);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &BufferData.VBO_Normal);
	glBindBuffer(GL_ARRAY_BUFFER, BufferData.VBO_Normal);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &BufferData.VBO_Height);
	glBindBuffer(GL_ARRAY_BUFFER, BufferData.VBO_Height);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);
	glEnableVertexAttribArray(3);

	if (UseEBO) {
		glGenBuffers(1, &BufferData.EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferData.EBO);
	}

	glBindVertexArray(0);
}

void BufferManager::BindVertexBufferObject(BufferData &BufferData, const glm::vec3* Position, int PositionSize, const glm::vec3* Color, int ColorSize,
										   const glm::vec3* Normals, const int NormalSize) {

	glBindVertexArray(BufferData.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, BufferData.VBO_Position);
	glBufferData(GL_ARRAY_BUFFER, PositionSize * sizeof(glm::vec3), Position, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, BufferData.VBO_Color);
	glBufferData(GL_ARRAY_BUFFER, ColorSize * sizeof(glm::vec3), Color, GL_STATIC_DRAW);


	if (Normals != nullptr && NormalSize > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, BufferData.VBO_Normal);
		glBufferData(GL_ARRAY_BUFFER, NormalSize * sizeof(glm::vec3), Normals, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
	}
	else {
		glDisableVertexAttribArray(2);
		glVertexAttrib3f(2, 0.0f, 0.0f, 0.0f); // Set default normal
	}

	std::vector<float> HeightData(PositionSize);
	for (int i = 0; i < PositionSize; i++) {HeightData[i] = Position[i].y;}

	glBindBuffer(GL_ARRAY_BUFFER, BufferData.VBO_Height);
	glBufferData(GL_ARRAY_BUFFER, PositionSize * sizeof(float), HeightData.data(), GL_DYNAMIC_DRAW);


	glBindVertexArray(0);

}

void BufferManager::BindElementBufferObject(BufferData &BufferData, const int* Index, int IndexSize) {

	if (BufferData.EBO == 0) return;

	glBindVertexArray(BufferData.VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferData.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexSize * sizeof(int), Index, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

BufferData& BufferManager::CreateBufferData(unsigned int ID, bool UseEBO) {

	BufferData& bd = BufferMap[ID];
	InitVertexArrayObejct(bd, UseEBO);
	return bd;

}

void BufferManager::BindVertexBufferObjectByID(unsigned int ID, const glm::vec3* Position, int PositionSize, const glm::vec3* Color, int ColorSize, const glm::vec3* Normal, int NormalSize) {
	
	auto it = BufferMap.find(ID);
	if (it == BufferMap.end()) {
		std::cerr << "Error: Buffer ID " << ID << " not found." << std::endl;
		return;
	}
	
	BufferData& bd = it->second;
	BindVertexBufferObject(bd, Position, PositionSize, Color, ColorSize, Normal, NormalSize);
}

void BufferManager::BindElementBufferObjectByID(unsigned int ID, const int* Index, int IndexSize) {

	auto it = BufferMap.find(ID);
	if (it == BufferMap.end()) {
		std::cerr << "Error: Buffer ID " << ID << " not found." << std::endl;
		return;

	}
	BufferData& bd = it->second;
	BindElementBufferObject(bd, Index, IndexSize);
}

void BufferManager::UploadHeight(BufferData& BufferData, const float* HeightMap, int Size) {

	glBindBuffer(GL_ARRAY_BUFFER, BufferData.VBO_Height);
	glBufferSubData(GL_ARRAY_BUFFER, 0, Size * sizeof(float), HeightMap);

}

void BufferManager::UploadHeightByID(unsigned int ID, const float* HeightMap, int Size) {

	auto it = BufferMap.find(ID);
	if (it == BufferMap.end()) {
		std::cerr << "Error: Buffer ID " << ID << " not found." << std::endl;
		return;
	}
	BufferData& bd = it->second;
	UploadHeight(bd, HeightMap, Size);

}

void BufferManager::UploadElevationTexture(int ResU, int ResV, 
										   const float* ElevationMap, const std::vector<glm::vec2>& NoiseMap, const uint8_t* ConstraintMaskMap, 
										   const int Index) {

	if (Textures[Index].Elevation.Texture[0]) glDeleteTextures(1, &Textures[Index].Elevation.Texture[0]);
	if (Textures[Index].Elevation.Texture[1]) glDeleteTextures(1, &Textures[Index].Elevation.Texture[1]);

	std::vector<glm::vec4> ElevationNoiseMap(ResU * ResV);
	for (int i = 0; i < ResU * ResV; i++) {
		ElevationNoiseMap[i] = glm::vec4(ElevationMap[i], NoiseMap[i].x, NoiseMap[i].y, float(ConstraintMaskMap[i]));
	}

	glGenTextures(1, &Textures[Index].Elevation.Texture[0]);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].Elevation.Texture[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ResU, ResV, 0, GL_RGBA, GL_FLOAT, ElevationNoiseMap.data());

	glGenTextures(1, &Textures[Index].Elevation.Texture[1]);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].Elevation.Texture[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ResU, ResV, 0, GL_RGBA, GL_FLOAT, ElevationNoiseMap.data());

}

void BufferManager::UploadGradientTexture(int ResU, int ResV, const std::vector<glm::vec3>& GradientMap, const int Index) {

	if (Textures[Index].Gradient.Texture[0]) glDeleteTextures(1, &Textures[Index].Gradient.Texture[0]);
	if (Textures[Index].Gradient.Texture[1]) glDeleteTextures(1, &Textures[Index].Gradient.Texture[1]);

	glGenTextures(1, &Textures[Index].Gradient.Texture[0]);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].Gradient.Texture[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	std::vector<GradientRGBA> GradientMapRGBA(ResU * ResV);
	for (int i = 0; i < ResU * ResV; i++) {
		GradientMapRGBA[i].nx = GradientMap[i].x;
		GradientMapRGBA[i].ny = GradientMap[i].y;
		GradientMapRGBA[i].norm = GradientMap[i].z;
		GradientMapRGBA[i].pad = 0.0f;
	}


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ResU, ResV, 0, GL_RGBA, GL_FLOAT, GradientMapRGBA.data());

	glGenTextures(1, &Textures[Index].Gradient.Texture[1]);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].Gradient.Texture[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ResU, ResV, 0, GL_RGBA, GL_FLOAT, GradientMapRGBA.data());

}


//void BufferManager::UploadNoiseTexture(int ResU, int ResV, const std::vector<glm::vec2>& NoiseMap, const int Index) {
//
//	if (Textures[Index].Noise.Texture[0]) glDeleteTextures(1, &Textures[Index].Noise.Texture[0]);
//	if (Textures[Index].Noise.Texture[1]) glDeleteTextures(1, &Textures[Index].Noise.Texture[1]);
//
//	glGenTextures(1, &Textures[Index].Noise.Texture[0]);
//	glBindTexture(GL_TEXTURE_2D, Textures[Index].Noise.Texture[0]);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, ResU, ResV, 0, GL_RG, GL_FLOAT, NoiseMap.data());
//
//	glGenTextures(1, &Textures[Index].Noise.Texture[1]);
//	glBindTexture(GL_TEXTURE_2D, Textures[Index].Noise.Texture[1]);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, ResU, ResV, 0, GL_RG, GL_FLOAT, NoiseMap.data());
//
//	
//
//}

//void BufferManager::UploadConstraintMaskTexture(int ResU, int ResV, const uint8_t* ConstraintMaskMap, const int Index) {
//
//	if (Textures[Index].ConstraintMask) glDeleteTextures(1, &Textures[Index].ConstraintMask);
//
//	glGenTextures(1, &Textures[Index].ConstraintMask);
//	glBindTexture(GL_TEXTURE_2D, Textures[Index].ConstraintMask);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, ResU, ResV, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, ConstraintMaskMap);
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
//}

void BufferManager::UploadDbgTexture(int ResU, int ResV, const int Index) {
	
	glGenTextures(1, &Textures[Index].DebugTexture);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].DebugTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ResU, ResV, 0, GL_RGBA, GL_FLOAT, nullptr);
}

void BufferManager::BindElevationTextureDiffusion(const int Index) {
	glBindImageTexture(0, Textures[Index].Elevation.GetReadTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, Textures[Index].Elevation.GetWriteTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}
// Diffusion 후 swap하기 때문에, Residual 계산할 때는 Old/New가 뒤집혀야 함, 또한 둘 다 ReadOnly로 바인딩
void BufferManager::BindElevationTextureResidual(const int Index) {
	glBindImageTexture(1, Textures[Index].Elevation.GetReadTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(0, Textures[Index].Elevation.GetWriteTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
}
void BufferManager::BindGradientTexture(const int Index) {
	glBindImageTexture(2, Textures[Index].Gradient.GetReadTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(3, Textures[Index].Gradient.GetWriteTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}
//void BufferManager::BindNoiseTexture(const int Index) {
//	glBindImageTexture(4, Textures[Index].Noise.GetReadTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
//	glBindImageTexture(5, Textures[Index].Noise.GetWriteTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
//}
//void BufferManager::BindConstraintMaskTexture(const int Index) {
//	glBindImageTexture(6, Textures[Index].ConstraintMask, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
//}
void BufferManager::BindGradientReadOnly(const int Index) {
	glBindImageTexture(2, Textures[Index].Gradient.GetReadTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
}

void BufferManager::UploadRasidualTexture(const int ResU, const int ResV, const int Index) {
	if (Textures[Index].Residual) glDeleteTextures(1, &Textures[Index].Residual);
	glGenTextures(1, &Textures[Index].Residual);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].Residual);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, ResU, ResV, 0, GL_RED, GL_FLOAT, nullptr);
}

void BufferManager::BindResidualTextureRead(const int Index) {

	glBindImageTexture(4, Textures[Index].Residual, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

}

void BufferManager::BindResidualTextureWrite(const int Index) {
	glBindImageTexture(3, Textures[Index].Residual, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
}

void BufferManager::BindDbgTexture(const int Index) {
	glBindImageTexture(7, Textures[Index].DebugTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}


//void BufferManager::BindTexturesDefault() {
//
//	glBindImageTexture(0, Textures.Elevation, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
//
//	glBindImageTexture(1, Textures.Gradient, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
//
//	glBindImageTexture(2, Textures.Noise, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG32F);
//
//	glBindImageTexture(3, Textures.ConstraintMask, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
//
//}

std::vector<float> BufferManager::ReadbackElevationTexture(int ResU, int ResV, const int Index) {
	std::vector<glm::vec4> ElevationDataRGBA(ResU * ResV);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].Elevation.GetReadTexture());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, ElevationDataRGBA.data());
	
	std::vector<float> ElevationData(ResU * ResV);
	for (int i = 0; i < ResU * ResV; i++) {
		ElevationData[i] = ElevationDataRGBA[i].x;
	}
	return ElevationData;
}

std::vector<glm::vec3> BufferManager::ReadbackElevationTextureVec3(int ResU, int ResV, const int Index) {
	std::vector<glm::vec4> ElevationDataVec4(ResU * ResV);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].Elevation.GetReadTexture());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, ElevationDataVec4.data());
	std::vector<glm::vec3> ElevationData(ResU * ResV);
	for (int i = 0; i < ResU * ResV; i++) {
		ElevationData[i] = glm::vec3(ElevationDataVec4[i].x, ElevationDataVec4[i].y, ElevationDataVec4[i].z);
	}
	return ElevationData;
}

std::vector<glm::vec4> BufferManager::ReadbackDbgTexture(int ResU, int ResV, const int Index) {
	std::vector<glm::vec4> DebugData(ResU * ResV);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].DebugTexture);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, DebugData.data());
	return DebugData;
}



std::vector<glm::vec3> BufferManager::ReadbackGradientTexture(int ResU, int ResV, const int Index) {
	std::vector<GradientRGBA> GradientDataRGBA(ResU * ResV);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].Gradient.GetReadTexture());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, GradientDataRGBA.data());

	std::vector<glm::vec3> GradientData(ResU * ResV);
	for (int i = 0; i < ResU * ResV; i++) {
		GradientData[i] = glm::vec3(GradientDataRGBA[i].nx, GradientDataRGBA[i].ny, GradientDataRGBA[i].norm);
	}
	return GradientData;
}

std::vector<glm::vec2> BufferManager::ReadbackNoiseTexture(int ResU, int ResV, const int Index) {
	std::vector<glm::vec4> NoiseDataRGBA(ResU * ResV);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].Elevation.GetReadTexture());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, NoiseDataRGBA.data());
	std::vector<glm::vec2> NoiseData(ResU * ResV);
	for (int i = 0; i < ResU * ResV; i++) {
		NoiseData[i] = glm::vec2(NoiseDataRGBA[i].y, NoiseDataRGBA[i].z);
	}
	return NoiseData;
}

std::vector<float> BufferManager::ReadbackResidualTexture(int ResU, int ResV, const int Index) {
	std::vector<glm::vec4> ResidualDataRGBA(ResU * ResV);
	glBindTexture(GL_TEXTURE_2D, Textures[Index].Residual);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, ResidualDataRGBA.data());
	std::vector<float> ResidualData(ResU * ResV);
	for (int i = 0; i < ResU * ResV; i++) {
		ResidualData[i] = ResidualDataRGBA[i].x;
	}
	return ResidualData;
}



void BufferManager::UnbindAllTextures() {

	GLint maxUnits = 0;
	glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxUnits);

	for (int i = 0; i < maxUnits; ++i)
	{
		glBindImageTexture(
			i,
			0,          // texture
			0,
			GL_FALSE,
			0,
			GL_READ_ONLY,
			GL_R32F     // format은 아무거나 OK
		);
	}
}

void BufferManager::UnbindElevationTexture(){
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
}

void BufferManager::CreateSSBO(const int Index) {
	glGenBuffers(1, &Textures[Index].SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, Textures[Index].SSBO);

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * 2, nullptr, GL_DYNAMIC_READ);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void BufferManager::BindSSBO(const int Index) {

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, Textures[Index].SSBO);

}

void BufferManager::UploadDebugPixel2(const GLuint ProgramID, const glm::ivec2& p0, const glm::ivec2& p1) {
	glUniform2i(glGetUniformLocation(ProgramID, "DebugPixel0"), p0.x, p0.y);
	glUniform2i(glGetUniformLocation(ProgramID, "DebugPixel1"), p1.x, p1.y);

}

void BufferManager::AskDebugPixel2(const GLuint ProgramID, const glm::ivec2& p0, const glm::ivec2& p1, const int Index) {
	BindSSBO(Index);
	UploadDebugPixel2(ProgramID, p0, p1);
}

void BufferManager::ReadPrintSSBO(const int Index) {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, Textures[Index].SSBO);

	float* data = (float*)glMapBuffer(
		GL_SHADER_STORAGE_BUFFER,
		GL_READ_ONLY
	);

	// debugData[0]
	printf("P0: F_L=%f F_G=%f Alpha=%f Beta=%f\n",
		data[0], data[1], data[2], data[3]
	);

	// debugData[1]
	printf("P1: F_L=%f F_G=%f Alpha=%f Beta=%f\n",
		data[4], data[5], data[6], data[7]
	);

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void BufferManager::AllocateTexture2D(GLuint& TextureID, int ResU, int ResV, GLenum InternalFormat, GLenum Format, GLenum Type, GLenum filter, GLenum wrap) {
	if (TextureID == 0) glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, ResU, ResV, 0, Format, Type, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);

}

void BufferManager::CreateDebugTextures(int ResU, int ResV) {
	DebugTexture.ResU = ResU;
	DebugTexture.ResV = ResV;

	AllocateTexture2D(DebugTexture.Tex0, ResU, ResV, GL_RGBA32F, GL_RGBA, GL_FLOAT);
	AllocateTexture2D(DebugTexture.Tex1, ResU, ResV, GL_RGBA32F, GL_RGBA, GL_FLOAT);
}

void BufferManager::UploadDebugTextures(const std::vector<glm::vec4>& PackedRGBA, const std::vector<glm::vec4>& PackedRGRC) {

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, DebugTexture.Tex0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DebugTexture.ResU, DebugTexture.ResV, GL_RGBA, GL_FLOAT, PackedRGBA.data());

	glBindTexture(GL_TEXTURE_2D, DebugTexture.Tex1);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DebugTexture.ResU, DebugTexture.ResV, GL_RGBA, GL_FLOAT, PackedRGRC.data());

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glBindTexture(GL_TEXTURE_2D, 0);

}

void BufferManager::BindDebugTextures(GLuint ShaderProgramID) {

	//printf("Tex0=%u isTex0=%d | Tex1=%u isTex1=%d\n",
	//	DebugTexture.Tex0, glIsTexture(DebugTexture.Tex0),
	//	DebugTexture.Tex1, glIsTexture(DebugTexture.Tex1));

	//glUseProgram(ShaderProgramID);

	/*GLint cur = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &cur);
	printf("BindDebugTextures: current=%d, arg=%d\n", cur, (int)ShaderProgramID);*/

	GLint loc0 = glGetUniformLocation(ShaderProgramID, "DebugTexture0");
	GLint loc1 = glGetUniformLocation(ShaderProgramID, "DebugTexture1");

	if (loc0 >= 0) glUniform1i(loc0, DebugTexture.unit0);
	if (loc1 >= 0) glUniform1i(loc1, DebugTexture.unit1);

	glActiveTexture(GL_TEXTURE0 + DebugTexture.unit0);
	glBindTexture(GL_TEXTURE_2D, DebugTexture.Tex0);

	glActiveTexture(GL_TEXTURE0 + DebugTexture.unit1);
	glBindTexture(GL_TEXTURE_2D, DebugTexture.Tex1);
}

void BufferManager::SwapElevation(const int Index) { Textures[Index].Elevation.Swap(); }
void BufferManager::SwapGradient(const int Index) { Textures[Index].Gradient.Swap(); }
//void BufferManager::SwapNoise(const int Index) { Textures[Index].Noise.Swap(); }
void BufferManager::ResetGradientPingPong(const int Index) { Textures[Index].Gradient.ping = 0; }
const GLuint& BufferManager::GetElevationTextureWrite(const int Index) { return Textures[Index].Elevation.GetWriteTexture(); }