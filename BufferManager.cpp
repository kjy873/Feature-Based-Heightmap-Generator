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