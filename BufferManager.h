#pragma once

#include <glew.h>
#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>

#include <iostream>

#include <glfw3.h>

class BufferManager
{
private:
	GLuint VBO;
	GLuint VAO;
	GLuint EBO;

public:

	BufferManager() {}
	~BufferManager() {}

	void InitBufferLine(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize);
	void InitBufferTriangle(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize);
	void InitBufferRectangle(GLuint& VAO, const glm::vec3* position, const int positionSize,
		const glm::vec3* color, const int colorSize,
		const int* index, const int indexSize, const glm::vec3* normals, const int normalSize);
};

