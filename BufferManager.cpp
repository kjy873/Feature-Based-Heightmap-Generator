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

void BufferManager::UploadElevationTexture(int ResU, int ResV, const float* ElevationMap) {

	if (Textures.Elevation.Texture[0]) glDeleteTextures(1, &Textures.Elevation.Texture[0]);
	if (Textures.Elevation.Texture[1]) glDeleteTextures(1, &Textures.Elevation.Texture[1]);

	glGenTextures(1, &Textures.Elevation.Texture[0]);
	glBindTexture(GL_TEXTURE_2D, Textures.Elevation.Texture[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, ResU, ResV, 0, GL_RED, GL_FLOAT, ElevationMap);

	glGenTextures(1, &Textures.Elevation.Texture[1]);
	glBindTexture(GL_TEXTURE_2D, Textures.Elevation.Texture[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, ResU, ResV, 0, GL_RED, GL_FLOAT, ElevationMap);

}

void BufferManager::UploadGradientTexture(int ResU, int ResV, const std::vector<glm::vec3>& GradientMap) {

	if (Textures.Gradient.Texture[0]) glDeleteTextures(1, &Textures.Gradient.Texture[0]);
	if (Textures.Gradient.Texture[1]) glDeleteTextures(1, &Textures.Gradient.Texture[1]);

	glGenTextures(1, &Textures.Gradient.Texture[0]);
	glBindTexture(GL_TEXTURE_2D, Textures.Gradient.Texture[0]);

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

	glGenTextures(1, &Textures.Gradient.Texture[1]);
	glBindTexture(GL_TEXTURE_2D, Textures.Gradient.Texture[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ResU, ResV, 0, GL_RGBA, GL_FLOAT, GradientMapRGBA.data());

}
void BufferManager::UploadNoiseTexture(int ResU, int ResV, const std::vector<glm::vec2>& NoiseMap) {

	if (Textures.Noise.Texture[0]) glDeleteTextures(1, &Textures.Noise.Texture[0]);
	if (Textures.Noise.Texture[1]) glDeleteTextures(1, &Textures.Noise.Texture[1]);

	glGenTextures(1, &Textures.Noise.Texture[0]);
	glBindTexture(GL_TEXTURE_2D, Textures.Noise.Texture[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, ResU, ResV, 0, GL_RG, GL_FLOAT, NoiseMap.data());

	glGenTextures(1, &Textures.Noise.Texture[1]);
	glBindTexture(GL_TEXTURE_2D, Textures.Noise.Texture[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, ResU, ResV, 0, GL_RG, GL_FLOAT, NoiseMap.data());

	

}

void BufferManager::UploadConstraintMaskTexture(int ResU, int ResV, const uint8_t* ConstraintMaskMap) {

	if (Textures.ConstraintMask) glDeleteTextures(1, &Textures.ConstraintMask);

	glGenTextures(1, &Textures.ConstraintMask);
	glBindTexture(GL_TEXTURE_2D, Textures.ConstraintMask);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, ResU, ResV, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, ConstraintMaskMap);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void BufferManager::UploadDebugTexture(int ResU, int ResV) {
	
	glGenTextures(1, &Textures.DebugTexture);
	glBindTexture(GL_TEXTURE_2D, Textures.DebugTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ResU, ResV, 0, GL_RGBA, GL_FLOAT, nullptr);
}

void BufferManager::BindElevationTexture() {

	//printf("Bind ReadTex = %u, WriteTex = %u\n",
	//	Textures.Elevation.GetReadTexture(),
	//	Textures.Elevation.GetWriteTexture());

	glBindImageTexture(0, Textures.Elevation.GetReadTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(1, Textures.Elevation.GetWriteTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
}
void BufferManager::BindGradientTexture() {
	glBindImageTexture(2, Textures.Gradient.GetReadTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(3, Textures.Gradient.GetWriteTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}
void BufferManager::BindNoiseTexture() {
	glBindImageTexture(4, Textures.Noise.GetReadTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
	glBindImageTexture(5, Textures.Noise.GetWriteTexture(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
}
void BufferManager::BindConstraintMaskTexture() {
	glBindImageTexture(6, Textures.ConstraintMask, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
}
void BufferManager::BindGradientReadOnly() {
	glBindImageTexture(2, Textures.Gradient.GetReadTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
}

void BufferManager::BindDebugTexture() {
	glBindImageTexture(7, Textures.DebugTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
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

std::vector<float> BufferManager::ReadbackElevationTexture(int ResU, int ResV) {
	std::vector<float> ElevationData(ResU * ResV);
	glBindTexture(GL_TEXTURE_2D, Textures.Elevation.GetReadTexture());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, ElevationData.data());
	/*for (const auto& height : ElevationData) {
		std::cout << height << std::endl;
	}*/
	return ElevationData;
}

std::vector<glm::vec3> BufferManager::ReadbackElevationTextureVec3(int ResU, int ResV) {
	std::vector<glm::vec4> ElevationDataVec4(ResU * ResV);
	glBindTexture(GL_TEXTURE_2D, Textures.Elevation.GetReadTexture());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, ElevationDataVec4.data());
	std::vector<glm::vec3> ElevationData(ResU * ResV);
	for (int i = 0; i < ResU * ResV; i++) {
		ElevationData[i] = glm::vec3(ElevationDataVec4[i].x, ElevationDataVec4[i].y, ElevationDataVec4[i].z);
	}
	return ElevationData;
}

std::vector<glm::vec4> BufferManager::ReadbackDebugTexture(int ResU, int ResV) {
	std::vector<glm::vec4> DebugData(ResU * ResV);
	glBindTexture(GL_TEXTURE_2D, Textures.DebugTexture);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, DebugData.data());
	return DebugData;
}



std::vector<glm::vec3> BufferManager::ReadbackGradientTexture(int ResU, int ResV) {
	std::vector<GradientRGBA> GradientDataRGBA(ResU * ResV);
	glBindTexture(GL_TEXTURE_2D, Textures.Gradient.GetReadTexture());
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, GradientDataRGBA.data());

	std::vector<glm::vec3> GradientData(ResU * ResV);
	for (int i = 0; i < ResU * ResV; i++) {
		GradientData[i] = glm::vec3(GradientDataRGBA[i].nx, GradientDataRGBA[i].ny, GradientDataRGBA[i].norm);
	}
	return GradientData;
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

void BufferManager::CreateSSBO() {
	glGenBuffers(1, &Textures.SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, Textures.SSBO);

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 4 * 2, nullptr, GL_DYNAMIC_READ);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void BufferManager::BindSSBO() {

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, Textures.SSBO);

}

void BufferManager::UploadDebugPixel2(const GLuint ProgramID, const glm::ivec2& p0, const glm::ivec2& p1) {
	glUniform2i(glGetUniformLocation(ProgramID, "DebugPixel0"), p0.x, p0.y);
	glUniform2i(glGetUniformLocation(ProgramID, "DebugPixel1"), p1.x, p1.y);

}

void BufferManager::AskDebugPixel2(const GLuint ProgramID, const glm::ivec2& p0, const glm::ivec2& p1) {
	BindSSBO();
	UploadDebugPixel2(ProgramID, p0, p1);
}

void BufferManager::ReadPrintSSBO() {

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, Textures.SSBO);

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