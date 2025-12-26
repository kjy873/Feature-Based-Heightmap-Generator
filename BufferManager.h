#pragma once

#include <glew.h>
#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>

#include <map>
#include <unordered_map>

#include <iostream>

#include <glfw3.h>

struct BufferData {
	GLuint VBO_Position;
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
	
	unsigned int BufferID = 0;

public:

	BufferManager() {}
	~BufferManager() {}

	void InitBufferLine(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize);
	void InitBufferTriangle(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize);
	void InitBufferRectangle(GLuint& VAO, const glm::vec3* position, const int positionSize,
		const glm::vec3* color, const int colorSize,
		const int* index, const int indexSize, const glm::vec3* normals, const int normalSize);

	void InitVertexArrayObejct(BufferData &BufferData, bool UseEBO);
	void BindVertexBufferObject(BufferData& BufferData, const glm::vec3* Position, int PositionSize, const glm::vec3* Color, int ColorSize, const glm::vec3* Normal, int NormalSize);
	void BindElementBufferObject(BufferData& BufferData, const int* Index, int IndexSize);

	unsigned int CreateMeshID() { return BufferID++; };

	BufferData& CreateBufferData(unsigned int ID, bool UseEBO);
	void BindVertexBufferObjectByID(unsigned int ID, const glm::vec3* Position, int PositionSize, const glm::vec3* Color, int ColorSize, const glm::vec3* Normal, int NormalSize);
	void BindElementBufferObjectByID(unsigned int ID, const int* Index, int IndexSize);

	GLuint& GetVAOByID(unsigned int ID) {
		auto it = BufferMap.find(ID);
		if (it == BufferMap.end()) {
			std::cerr << "Error: Buffer ID " << ID << " not found." << std::endl;
			throw std::runtime_error("Buffer ID not found");
		}
		return it->second.VAO;
	}
};

