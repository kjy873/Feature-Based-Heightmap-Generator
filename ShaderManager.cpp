#include "ShaderManager.h"

std::string ShaderManager::FileToBuf(const char* filepath)
{
	std::ifstream file(filepath);
	if (!file.is_open()) {
		throw std::runtime_error(
			std::string("Failed to open file: ") + filepath
		);
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}


void ShaderManager::make_vertexShaders(GLuint& vertexShader, const char* vertexName) {
	
	std::string Source = FileToBuf(vertexName);
	
	//GLchar* vertexSource;

	const char* vertexSource = Source.c_str();

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
	std::string Source = FileToBuf(fragmentName);

	const char* fragmentSource = Source.c_str();

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

bool ShaderManager::Make_ComputeShaders(GLuint& computeShader, const char* computeName) {
	
	std::string Source = FileToBuf(computeName);

	const char* ComputeSource = Source.c_str();

	GLuint Shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(Shader, 1, &ComputeSource, NULL);
	glCompileShader(Shader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(Shader, 512, NULL, errorLog);
		std::cerr << "ERROR: compute shader error\n" << errorLog << std::endl;
		glDeleteShader(Shader);
		return false;

	}

	computeShader = Shader;
	return true;

}
bool ShaderManager::Make_ComputeProgram(GLuint& shaderProgramID, GLuint& computeShader) {
	
	GLuint ProgramID = glCreateProgram();

	glAttachShader(ProgramID, computeShader);

	glLinkProgram(ProgramID);

	glDeleteShader(computeShader);

	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(ProgramID, 512, NULL, errorLog);
		std::cerr << "ERROR: compute shader program 연결 실패\n" << errorLog << std::endl;
		return false;
	}
	
	shaderProgramID = ProgramID;
	return true;
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

GLvoid ShaderManager::InitComputePrograms(const char* GradientName, const char* ElevationName, const char* NoiseName, const char* MultigridName) {

	ComputeProgram GradientProgram, ElevationProgram, NoiseProgram, MultigridProgram;
	
	if(!Make_ComputeShaders(GradientProgram.ComputeShader, GradientName)) return;
	if(!Make_ComputeProgram(GradientProgram.Program, GradientProgram.ComputeShader)) return;

	if(!Make_ComputeShaders(ElevationProgram.ComputeShader, ElevationName)) return;
	if(!Make_ComputeProgram(ElevationProgram.Program, ElevationProgram.ComputeShader)) return;

	if(!Make_ComputeShaders(NoiseProgram.ComputeShader, NoiseName)) return;
	if(!Make_ComputeProgram(NoiseProgram.Program, NoiseProgram.ComputeShader)) return;

	if(!Make_ComputeShaders(MultigridProgram.ComputeShader, MultigridName)) return;
	if(!Make_ComputeProgram(MultigridProgram.Program, MultigridProgram.ComputeShader)) return;

	std::cout << "성공성공" << std::endl;

	ComputeShaders.insert({ ComputeType::Gradient, GradientProgram });
	ComputeShaders.insert({ ComputeType::Elevation, ElevationProgram });
	ComputeShaders.insert({ ComputeType::Noise, NoiseProgram });
	ComputeShaders.insert({ ComputeType::Multigrid, MultigridProgram });

}