#pragma once

#include <glew.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>

#include <unordered_map>

#include <glfw3.h>

enum class ComputeType {
	Gradient,
	Elevation,
	Noise,
	Multigrid,
	Residual
};

struct ComputeProgram {
	GLuint Program = 0;
	GLuint ComputeShader = 0;

	void Use() const { glUseProgram(Program); }
};

class ShaderManager
{



private:
	GLuint ShaderProgramID = 0;
	GLuint VertexShader = 0;
	GLuint FragmentShader = 0;


	std::unordered_map<ComputeType, ComputeProgram> ComputeShaders;

	const char* VertexName = NULL;
	const char* FragmentName = NULL;

public:

	ShaderManager(const char* VertexName, const char* FragmentName) : VertexName(VertexName), FragmentName(FragmentName){}
	~ShaderManager() {}

	// init shader
	std::string FileToBuf(const char* filepath);

	void make_vertexShaders(GLuint& vertexShader, const char* vertexName);

	void make_fragmentShaders(GLuint& fragmentShader, const char* fragmentName);

	void make_shaderProgram(GLuint& shaderProgramID, GLuint& vertexShader, GLuint& fragmentShader);

	bool Make_ComputeShaders(GLuint& computeShader, const char* computeName);

	bool Make_ComputeProgram(GLuint& shaderProgramID, GLuint& computeShader);

	GLvoid InitShader();
	GLvoid InitComputePrograms(const char* GradientName, const char* ElevationName, const char* NoiseName, const char* MultigridName);
	GLvoid AddComputeShaderProgram(const char* ComputeName, const ComputeType Type);
	// init buffer

	const GLuint& GetShaderProgramID() const{ return ShaderProgramID; }
	const GLuint& GetVertexShader() { return VertexShader; }
	const GLuint& GetFragmentShader() { return FragmentShader; }



	ComputeProgram& FindComputeProgram(ComputeType Type) {
		return ComputeShaders.at(Type);
	}
};




