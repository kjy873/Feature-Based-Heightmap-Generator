#include "ShaderManager.h"


GLchar* ShaderManager::filetobuf(const char* filepath)
{
	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filepath << std::endl;
		return nullptr;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	std::string contents = buffer.str();
	char* source = new char[contents.size() + 1];
	strcpy_s(source, contents.size() + 1, contents.c_str());
	return source;
}

void ShaderManager::make_vertexShaders(GLuint& vertexShader, const char* vertexName) {
	GLchar* vertexSource;

	vertexSource = filetobuf(vertexName);

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader error\n" << errorLog << std::endl;
		return;
	}
}

void ShaderManager::make_fragmentShaders(GLuint& fragmentShader, const char* fragmentName) {
	GLchar* fragmentSource;

	fragmentSource = filetobuf(fragmentName);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader error\n" << errorLog << std::endl;
		return;
	}
}

void ShaderManager::Make_ComputeShaders(GLuint& computeShader, const char* computeName) {
	GLchar* ComputeSource;

	ComputeSource = filetobuf(computeName);

	computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &ComputeSource, NULL);
	glCompileShader(computeShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(computeShader, 512, NULL, errorLog);
		std::cerr << "ERROR: compute shader error\n" << errorLog << std::endl;
		return;
	}

}

void ShaderManager::Make_ComputeProgram(GLuint& shaderProgramID, GLuint& computeShader) {
	shaderProgramID = glCreateProgram();

	glAttachShader(shaderProgramID, computeShader);

	glLinkProgram(shaderProgramID);

	glDeleteShader(computeShader);

	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderProgramID, 512, NULL, errorLog);
		std::cerr << "ERROR: compute shader program 연결 실패\n" << errorLog << std::endl;
		return;
	}
	
}

void ShaderManager::make_shaderProgram(GLuint& shaderProgramID, GLuint& vertexShader, GLuint& fragmentShader) {
	shaderProgramID = glCreateProgram();

	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);

	glLinkProgram(shaderProgramID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderProgramID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return;
	}
}
GLvoid ShaderManager::InitShader() {
	make_vertexShaders(VertexShader, VertexName);
	make_fragmentShaders(FragmentShader, FragmentName);
	make_shaderProgram(ShaderProgramID, VertexShader, FragmentShader);
}