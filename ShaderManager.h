#pragma once

#include <glew.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>

#include <glfw3.h>

class ShaderManager
{



private:
	GLuint ShaderProgramID;
	GLuint VertexShader;
	GLuint FragmentShader;

	const char* VertexName = NULL;
	const char* FragmentName = NULL;

public:

	ShaderManager(const char* VertexName, const char* FragmentName) : VertexName(VertexName), FragmentName(FragmentName){}
	~ShaderManager() {}

	// init shader
	GLchar* filetobuf(const char* filepath);

	void make_vertexShaders(GLuint& vertexShader, const char* vertexName);

	void make_fragmentShaders(GLuint& fragmentShader, const char* fragmentName);

	void make_shaderProgram(GLuint& shaderProgramID, GLuint& vertexShader, GLuint& fragmentShader);

	GLvoid InitShader();
	// init buffer

	GLuint& GetShaderProgramID() { return ShaderProgramID; }
	GLuint& GetVertexShader() { return VertexShader; }
	GLuint& GetFragmentShader() { return FragmentShader; }
};




