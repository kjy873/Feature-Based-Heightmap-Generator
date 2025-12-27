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
	GLuint ShaderProgramID = 0;
	GLuint VertexShader = 0;
	GLuint FragmentShader = 0;

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

	const GLuint& GetShaderProgramID() const{ return ShaderProgramID; }
	const GLuint& GetVertexShader() { return VertexShader; }
	const GLuint& GetFragmentShader() { return FragmentShader; }
};




