#define _CRT_SECURE_NO_WARININGS
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <random>
#include <algorithm>
#include <sstream>
#include <string.h>
#include <string>

using namespace std;

bool MoveCameraForward = false;
bool MoveCameraBackward = false;
bool MoveCameraLeft = false;
bool MoveCameraRight = false;
float CameraYaw = -90.0f;
float CameraPitch = 0.0f;

float windowWidth = 800;
float windowHeight = 600;
const float defaultSize = 0.05;

// one bezier curve by 4 control points
vector<glm::vec3> bezier(glm::vec3 ControlPoints[4]) {

	glm::vec3 P = glm::vec3(0.0f, 0.0f, 0.0f);

	vector<glm::vec3> bezierPoints;

	for (float t = 0; t < 1; t += 0.01f) {
		P = (1 - t) * (1 - t) * (1 - t) * ControlPoints[0] +
			3 * (1 - t) * (1 - t) * t * ControlPoints[1] +
			3 * (1 - t) * t * t * ControlPoints[2] +
			t * t * t * ControlPoints[3];
		//std::cout << "P: " << P.x << ", " << P.y << ", " << P.z << std::endl;
		bezierPoints.push_back(P);
	}

	return bezierPoints;
}

struct COLOR {
	GLclampf R = 1.0f;
	GLclampf G = 1.0f;
	GLclampf B = 1.0f;
	GLclampf A = 0.0f;
};
GLvoid setColor(COLOR& c, GLclampf r, GLclampf g, GLclampf b, GLclampf a) {
	c.R = r;
	c.G = g;
	c.B = b;
	c.A = a;
}
GLvoid setColorRand(COLOR& c) {
	c.R = (float)(rand() % 256 + 1) / 255;
	c.G = (float)(rand() % 256 + 1) / 255;
	c.B = (float)(rand() % 256 + 1) / 255;
	c.A = 1.0f;

}
glm::vec3* returnColorRand2() {
	glm::vec3 color[2];
	for (int i = 0; i < 2; i++) color[i] = glm::vec3((float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255);
	return color;
}
glm::vec3* returnColorRand8() {
	glm::vec3 color[8];
	for (int i = 0; i < 8; i++) color[i] = glm::vec3((float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255);
	return color;
}
glm::vec3* returnColor8(const glm::vec3 c) {
	glm::vec3 color[8];
	for (int i = 0; i < 8; i++) color[i] = c;
	return color;
}
COLOR backgroundColor{ 1.0f, 1.0f, 1.0f, 0.0f };

// mouse point to GL coordinate
struct mouseLocationGL {
	float x;
	float y;
};
mouseLocationGL transformMouseToGL(int x, int y) {
	mouseLocationGL m;
	m.x = (2.0f * x) / windowWidth - 1.0f;
	m.y = 1.0f - (2.0f * y) / windowHeight;
	return m;
}
mouseLocationGL mgl;
mouseLocationGL preMousePosition;


void idleScene();
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid KeyboardUp(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void init();
void motion(int x, int y);
GLvoid CameraTimer(int value);
GLchar* filetobuf(const char* filepath);
void make_vertexShaders(GLuint& vertexShader, const char* vertexName);
void make_fragmentShaders(GLuint& fragmentShader, const char* fragmentName);
void make_shaderProgram(GLuint& shaderProgramID);
void InitBufferLine(GLuint& VAO, const glm::vec3* position, int positionSize,
	const glm::vec3* color, int colorSize);
void InitBufferTriangle(GLuint& VAO, const glm::vec3* position, int positionSize,
	const glm::vec3* color, int colorSize);
void InitBufferRectangle(GLuint& VAO, const glm::vec3* position, const int positionSize,
	const glm::vec3* color, const int colorSize,
	const int* index, const int indexSize, const glm::vec3* normals, const int normalSize);
inline GLvoid InitShader(GLuint& programID, GLuint& vertex, const char* vertexName, GLuint& fragment, const char* fragmentName);

// shape struct
struct diagram {
	GLuint VAO{ NULL };												// VAO
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);					// center
	float radius = defaultSize;										// radius
	glm::vec3 initialCenter = glm::vec3(0.0f, 0.0f, 0.0f);			// initial center
	vector<glm::vec3> position;										// vertex positions
	vector<glm::vec3> currentPosition;								// current vertex positions
	vector<glm::vec3> color;										// vertex colors
	vector<glm::vec3> normals;									    // vertex normal vectors
	vector<glm::vec3> currentNormals;								// current normal vectors
	int vertices = 0;												// number of vertices
	bool polyhedron = false;										// polyhedron?
	vector<int> index;												// index for EBO
	glm::mat4 TSR = glm::mat4(1.0f);								// transform matrix
	float width{ 0 };
	float height{ 0 };
	float depth{ 0 };

	//2D shape
	diagram(int vertexCount) : vertices(vertexCount) {
		position.resize(vertices);
		currentPosition.resize(vertices);
		color.resize(vertices);
		if (vertices == 4) {
			index.resize(6);
			index = vector<int>{ 0, 1, 3, 1, 2, 3 };
		}
	}
};

// shape: init line
void setLine(diagram& dst, const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3* c) {
	for (int i = 0; i < 2; i++) {
		dst.color[i] = glm::vec3(c[i]);
	}
	dst.position[0] = glm::vec3(vertex1);
	dst.position[1] = glm::vec3(vertex2);

	InitBufferLine(dst.VAO, dst.position.data(), dst.position.size(), dst.color.data(), dst.color.size());
}

// move
inline void move(diagram& dia, glm::vec3 delta) {
	dia.TSR = glm::translate(glm::mat4(1.0f), delta) * dia.TSR;
	dia.center = glm::vec3(dia.TSR * glm::vec4(dia.initialCenter, 1.0f));
	for (int i = 0; i < dia.position.size(); i++) dia.currentPosition[i] = glm::vec3(dia.TSR * glm::vec4(dia.position[i], 1.0f));
	for (int i = 0; i < dia.currentNormals.size(); i++) dia.currentNormals[i] = glm::vec3(dia.TSR * glm::vec4(dia.normals[i], 1.0f));
}
// rotate
inline void rotateByCenter(diagram& dia, glm::vec3 axis, const float& degree) {
	dia.TSR = glm::translate(glm::mat4(1.0f), -dia.center) * dia.TSR;
	dia.TSR = glm::rotate(glm::mat4(1.0f), glm::radians(degree), glm::normalize(axis)) * dia.TSR;
	dia.TSR = glm::translate(glm::mat4(1.0f), dia.center) * dia.TSR;
	dia.center = glm::vec3(dia.TSR * glm::vec4(dia.initialCenter, 1.0f));
	for (int i = 0; i < dia.position.size(); i++) dia.currentPosition[i] = glm::vec3(dia.TSR * glm::vec4(dia.position[i], 1.0f));
	for (int i = 0; i < dia.currentNormals.size(); i++) dia.currentNormals[i] = glm::vec3(dia.TSR * glm::vec4(dia.normals[i], 1.0f));
}
// orbit
inline void moveAndRotate(diagram& dia, glm::vec3 axis, glm::vec3 delta, const float& degree) {
	dia.TSR = glm::translate(glm::mat4(1.0f), -(dia.center + delta)) * dia.TSR;
	dia.TSR = glm::rotate(glm::mat4(1.0f), glm::radians(degree), glm::vec3(axis)) * dia.TSR;
	dia.TSR = glm::translate(glm::mat4(1.0f), dia.center + delta) * dia.TSR;
	dia.center = glm::vec3(dia.TSR * glm::vec4(dia.initialCenter, 1.0f));
	for (int i = 0; i < dia.position.size(); i++) dia.currentPosition[i] = glm::vec3(dia.TSR * glm::vec4(dia.position[i], 1.0f));
	for (int i = 0; i < dia.currentNormals.size(); i++) dia.currentNormals[i] = glm::vec3(dia.TSR * glm::vec4(dia.normals[i], 1.0f));
}
// camera orbit
inline void moveAndRotateByMatrix(glm::mat4& TSR, glm::vec3 axis, glm::vec3 center, glm::vec3 moving, const float& degree) {
	TSR = glm::translate(TSR, -(center + moving)) * TSR;
	TSR = glm::rotate(TSR, glm::radians(degree), glm::vec3(axis)) * TSR;
	TSR = glm::translate(TSR, (center + moving)) * TSR;
}
// scale
inline void scaleByCenter(diagram& dia, glm::vec3 size) {
	glm::vec3 preLocation = glm::vec3(dia.center);
	move(dia, -dia.center);
	dia.TSR = glm::scale(glm::mat4(1.0f), size) * dia.TSR;
	move(dia, preLocation);
	dia.center = glm::vec3(dia.TSR * glm::vec4(dia.initialCenter, 1.0f));
	for (int i = 0; i < dia.position.size(); i++) dia.currentPosition[i] = glm::vec3(dia.TSR * glm::vec4(dia.position[i], 1.0f));
}
// normal vector of a, b
glm::vec3 getNormal(const glm::vec3& a, const glm::vec3& b) {
	return glm::normalize(glm::cross(a, b));
}

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;
vector <diagram> axes;
// camera
glm::vec3 camera[3];
glm::vec3 CameraForward;
glm::vec3 CameraRight;
glm::mat4 view = glm::mat4(1.0f);

vector<vector<diagram>> FeatureCurves;


void main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Example1");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cerr << "unable to initialize GLEW" << endl;
		exit(EXIT_FAILURE);
	}
	else cout << "GLEW initialized" << endl;

	InitShader(shaderProgramID, vertexShader, "vertex.glsl", fragmentShader, "fragment.glsl");
	glUseProgram(shaderProgramID);

	init();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyboardUp);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(10, CameraTimer, 0);

	glutMainLoop();
}





void init() {
	glClearColor(backgroundColor.R, backgroundColor.G, backgroundColor.B, backgroundColor.A);
	
	// axes
	diagram* temp = new diagram(2);
	setLine(*temp, glm::vec3(-4.0, 0.0, 0.0), glm::vec3(4.0, 0.0, 0.0), returnColorRand2());
	axes.push_back(*temp);
	setLine(*temp, glm::vec3(0.0, -4.0, 0.0), glm::vec3(0.0, 4.0, 0.0), returnColorRand2());
	axes.push_back(*temp);
	setLine(*temp, glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 0.0, 4.0), returnColorRand2());
	axes.push_back(*temp);
	delete(temp);

	camera[0] = glm::vec3(-0.1, 0.2, 0.3);
	camera[1] = glm::vec3(0.0, 0.0, 0.0);
	camera[2] = glm::vec3(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(camera[0], camera[1], camera[2]);
	CameraForward = camera[1] - camera[0];;

	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ Generate feature curve ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	glm::vec3 ControlPoints[4] = { 
		glm::vec3(-1.0f, -1.0f, 0.0f),
		glm::vec3(-0.3333f, 1.0f, 1.0f), 
		glm::vec3(0.3333f, 1.0f, -1.0f), 
		glm::vec3(1.0f, -1.0f, 0.0f) 
	};

	vector<glm::vec3> bezierPoints = bezier(ControlPoints);  // points for bezier curve
	vector<diagram> bezierLines; // lines for bezier curve
	
	diagram* line = new diagram(2);
	for (auto i = bezierPoints.begin(); i != bezierPoints.end() - 1; i++) {
		setLine(*line, *i, *(i + 1), returnColorRand2());
		bezierLines.push_back(*line);
	}
	delete(line);

	FeatureCurves.push_back(bezierLines);
	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

	glEnable(GL_DEPTH_TEST);
}

void idleScene() {
	glutPostRedisplay();
}

inline void draw(const vector<diagram> dia) {
	for (const auto& d : dia) {
		glBindVertexArray(d.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(d.TSR));
		if (d.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
		else if (d.vertices == 3) glDrawArrays(GL_TRIANGLES, 0, 3);
		else if (d.vertices == 4) glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		else if (d.vertices == 5 && d.polyhedron) glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
		else if (d.vertices == 8 && d.polyhedron) glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}
}
inline void drawWireframe(const vector<diagram> dia) {
	for (const auto& d : dia) {
		glBindVertexArray(d.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(d.TSR));
		if (d.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
		else if (d.vertices == 3) glDrawArrays(GL_LINE_LOOP, 0, 3);
		else if (d.vertices == 4) glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0);
		else if (d.vertices == 5 && d.polyhedron) glDrawElements(GL_LINE_LOOP, 18, GL_UNSIGNED_INT, 0);
		else if (d.vertices == 8 && d.polyhedron) glDrawElements(GL_LINE_LOOP, 36, GL_UNSIGNED_INT, 0);
	}
}
inline void draw(const diagram& dia) {
	glBindVertexArray(dia.VAO);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(dia.TSR));
	if (dia.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
	else if (dia.vertices == 3) glDrawArrays(GL_TRIANGLES, 0, 3);
	else if (dia.vertices == 4) glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	else if (dia.vertices == 5 && dia.polyhedron) glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
	else if (dia.vertices == 8 && dia.polyhedron) glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}
inline void drawWireframe(const diagram& dia) {
	glBindVertexArray(dia.VAO);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(dia.TSR));
	if (dia.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
	else if (dia.vertices == 3) glDrawArrays(GL_LINE_LOOP, 0, 3);
	else if (dia.vertices == 4) glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0);
	else if (dia.vertices == 5 && dia.polyhedron) glDrawElements(GL_LINE_LOOP, 18, GL_UNSIGNED_INT, 0);
	else if (dia.vertices == 8 && dia.polyhedron) glDrawElements(GL_LINE_LOOP, 36, GL_UNSIGNED_INT, 0);
}

GLvoid drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw(axes);
	view = glm::lookAt(camera[0], camera[0] + CameraForward, camera[2]);
	for (const auto& c : FeatureCurves) {
		draw(c);
	}

	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "viewTransform");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
	projection = glm::translate(projection, glm::vec3(0.0, 0.0, -2.0));
	unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

	GLuint trans_mat = glGetUniformLocation(shaderProgramID, "modelTransform");
	GLuint color = glGetUniformLocation(shaderProgramID, "vColor");

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		MoveCameraForward = true;
		break;
	case 's':
		MoveCameraBackward = true;
		break;
	case 'a':
		MoveCameraLeft = true;
		break;
	case 'd':
		MoveCameraRight = true;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

GLvoid KeyboardUp(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		MoveCameraForward = false;
		break;
	case 's':
		MoveCameraBackward = false;
		break;
	case 'a':
		MoveCameraLeft = false;
		break;
	case 'd':
		MoveCameraRight = false;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	mgl = transformMouseToGL(x, y);
	
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && state) {
		glutMotionFunc(motion);
	}

	preMousePosition = mgl;
	glutPostRedisplay();
}

void motion(int x, int y) {
	mgl = transformMouseToGL(x, y);
	
	float sensitivity = 100.0f;
	float dx = mgl.x - preMousePosition.x;
	float dy = mgl.y - preMousePosition.y;

	CameraYaw += dx * sensitivity;
	CameraPitch += dy * sensitivity;

	if (CameraPitch > 89.0f) CameraPitch = 89.0f;
	if (CameraPitch < -89.0f) CameraPitch = -89.0f;

	CameraForward.x = cos(glm::radians(CameraYaw)) * cos(glm::radians(CameraPitch));
	CameraForward.y = sin(glm::radians(CameraPitch));
	CameraForward.z = sin(glm::radians(CameraYaw)) * cos(glm::radians(CameraPitch));
	CameraForward = glm::normalize(CameraForward);

	CameraRight = glm::normalize(glm::cross(CameraForward, camera[2]));

	preMousePosition = mgl;
	glutPostRedisplay();
}

GLvoid CameraTimer(int value) {
	if (MoveCameraForward) {
		camera[0] += CameraForward * 0.01f;
	}

	if (MoveCameraBackward) {
		camera[0] -= CameraForward * 0.01f;
	}

	if (MoveCameraLeft) {
		camera[0] -= CameraRight * 0.01f;
	}

	if (MoveCameraRight) {
		camera[0] += CameraRight * 0.01f;
	}

	glutTimerFunc(10, CameraTimer, 0);
}

// init shader
GLchar* filetobuf(const char* filepath)
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
void make_vertexShaders(GLuint& vertexShader, const char* vertexName) {
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
		cerr << "ERROR: vertex shader error\n" << errorLog << endl;
		return;
	}
}
void make_fragmentShaders(GLuint& fragmentShader, const char* fragmentName) {
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
		cerr << "ERROR: fragment shader error\n" << errorLog << endl;
		return;
	}
}
void make_shaderProgram(GLuint& shaderProgramID) {
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
		cerr << "ERROR: shader program 연결 실패\n" << errorLog << endl;
		return;
	}
}
inline GLvoid InitShader(GLuint& programID, GLuint& vertex, const char* vertexName, GLuint& fragment, const char* fragmentName) {
	make_vertexShaders(vertexShader, vertexName);
	make_fragmentShaders(fragmentShader, fragmentName);
	make_shaderProgram(shaderProgramID);
}
// init buffer
void InitBufferLine(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize) {
	cout << "버퍼 초기화" << endl;
	GLuint VBO_position, VBO_color;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);

	glBufferData(GL_ARRAY_BUFFER, 3 * positionSize * sizeof(float), position, GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(shaderProgramID, "vPos");
	if (pAttribute < 0) cout << "pAttribute < 0" << endl;
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);

	glBufferData(GL_ARRAY_BUFFER, 3 * colorSize * sizeof(float), color, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(shaderProgramID, "vColor");
	if (cAttribute < 0) cout << "cAttribute < 0" << endl;
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);
}
void InitBufferTriangle(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize) {
	cout << "버퍼 초기화" << endl;
	GLuint VBO_position, VBO_color;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);

	glBufferData(GL_ARRAY_BUFFER, 3 * positionSize * sizeof(float), position, GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(shaderProgramID, "vPos");
	if (pAttribute < 0) cout << "pAttribute < 0" << endl;
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);

	glBufferData(GL_ARRAY_BUFFER, 3 * colorSize * sizeof(float), color, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(shaderProgramID, "vColor");
	if (cAttribute < 0) cout << "cAttribute < 0" << endl;
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);
}
void InitBufferRectangle(GLuint& VAO, const glm::vec3* position, const int positionSize,
	const glm::vec3* color, const int colorSize,
	const int* index, const int indexSize, const glm::vec3* normals, const int normalSize) {
	//cout << "EBO 초기화" << endl;
	GLuint VBO_position, VBO_color, NBO, EBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glBufferData(GL_ARRAY_BUFFER, 3 * positionSize * sizeof(float), position, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(int), index, GL_STATIC_DRAW);

	GLint pAttribute = glGetAttribLocation(shaderProgramID, "vPos");
	if (pAttribute < 0) cout << "pAttribute < 0" << endl;
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);

	glBufferData(GL_ARRAY_BUFFER, 3 * colorSize * sizeof(float), color, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(shaderProgramID, "vColor");
	if (cAttribute < 0) cout << "cAttribute < 0" << endl;
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);

}