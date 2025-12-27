#include "BufferManager.h"

void BufferManager::InitBufferLine(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize) {
	//cout << "버퍼 초기화" << endl;
	GLuint VBO_position, VBO_color;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);

	glBufferData(GL_ARRAY_BUFFER, 3 * positionSize * sizeof(float), position, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);

	glBufferData(GL_ARRAY_BUFFER, 3 * colorSize * sizeof(float), color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
}

void BufferManager::InitBufferTriangle(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize) {
	//cout << "버퍼 초기화" << endl;
	GLuint VBO_position, VBO_color;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);

	glBufferData(GL_ARRAY_BUFFER, 3 * positionSize * sizeof(float), position, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);

	glBufferData(GL_ARRAY_BUFFER, 3 * colorSize * sizeof(float), color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
}

void BufferManager::InitBufferRectangle(GLuint& VAO, const glm::vec3* position, const int positionSize,
	const glm::vec3* color, const int colorSize,
	const int* index, const int indexSize, const glm::vec3* normals, const int normalSize) {
	//cout << "EBO 초기화" << endl;
	GLuint VBO_position, VBO_color, VBO_normal, EBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glBufferData(GL_ARRAY_BUFFER, 3 * positionSize * sizeof(float), position, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glBufferData(GL_ARRAY_BUFFER, 3 * colorSize * sizeof(float), color, GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &VBO_normal);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_normal);
	glBufferData(GL_ARRAY_BUFFER, 3 * normalSize * sizeof(float), normals, GL_STATIC_DRAW);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(unsigned int), index, GL_STATIC_DRAW);

}

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