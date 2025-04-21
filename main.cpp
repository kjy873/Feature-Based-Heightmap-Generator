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

#define RESULUTION 1024

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
// sampling by 0.01
// this curve defines the directional flow of the terrain it represents
// During curve creation, the elevation component is initially set to zero, 
// and the actual terrain height is later diffused or interpolated based on the elevation constraints of the control points
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

glm::vec3 pointOnBezier(glm::vec3 ControlPoints[4], float u) {
	glm::vec3 P = glm::vec3(0.0f, 0.0f, 0.0f);
	P = (1 - u) * (1 - u) * (1 - u) * ControlPoints[0] +
		3 * (1 - u) * (1 - u) * u * ControlPoints[1] +
		3 * (1 - u) * u * u * ControlPoints[2] +
		u * u * u * ControlPoints[3];
	return P;
}

void normalize(glm::vec3& v) {
	v = (v / static_cast<float>(RESULUTION)) * 2.0f - 1.0f;
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

glm::vec3* returnColorRand2() {
	glm::vec3 color[2];
	for (int i = 0; i < 2; i++) color[i] = glm::vec3((float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255);
	return color;
}

COLOR backgroundColor{ 1.0f, 1.0f, 1.0f, 0.0f };

// two constraint points at least are attached to a curve, at extremities
// height, radius, gradient constraint range, gradient constraint angle, position on curve attached and noise parameters(A, R)
// constraints can include any combination of the six elements except for u (position), which is always required.
// since there are 64 possible combinations, an 8-bit flag is used to represent the presence or absence of each element
// there is no separate constructor or factory function — values are directly assigned to the constraint elements at the time of creation
// if certain combinations of flags are used frequently, helper functions can be created for convenience
struct ConstraintPoint {
	uint8_t flag;
	float h, r, a, b, alpha, beta, u, A, R;
	enum Flag {
		HAS_H = 1 << 0, 
		HAS_R = 1 << 1,
		HAS_A = 1 << 2,
		HAS_B = 1 << 3,
		HAS_ALPHA = 1 << 4,
		HAS_BETA = 1 << 5,
		HAS_AMPLITUDE = 1 << 6,
		HAS_RESPONSE = 1 << 7
	};
};

// linear interpolation
float lerp(float a, float b, float t) {
	return (1 - t) * a + t * b;
}

// hold interpolation
float hold(float p, float t) {
	return (1 - t) * p + t * p;
}

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
struct Shape {
	GLuint VAO{ NULL };												// VAO
	vector<glm::vec3> position;										// vertex positions
	vector<glm::vec3> currentPosition;								// current vertex positions
	vector<glm::vec3> color;										// vertex colors
	int vertices = 0;												// number of vertices
	glm::mat4 TSR = glm::mat4(1.0f);								// transform matrix
	vector<ConstraintPoint> CP;										// constraint points

	//2D shape
	Shape(int vertexCount) : vertices(vertexCount) {
		position.resize(vertices);
		currentPosition.resize(vertices);
		color.resize(vertices);
	}
};

// shape: init line
void setLine(Shape& dst, const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3* c) {
	for (int i = 0; i < 2; i++) {
		dst.color[i] = glm::vec3(c[i]);
	}
	dst.position[0] = glm::vec3(vertex1);
	dst.position[1] = glm::vec3(vertex2);

	InitBufferLine(dst.VAO, dst.position.data(), dst.position.size(), dst.color.data(), dst.color.size());
}

// normal vector of a, b
glm::vec3 getNormal(const glm::vec3& a, const glm::vec3& b) {
	return glm::normalize(glm::cross(a, b));
}

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;
vector <Shape> axes;
// camera
glm::vec3 camera[3];
glm::vec3 CameraForward;
glm::vec3 CameraRight;
glm::mat4 view = glm::mat4(1.0f);

vector<vector<Shape>> FeatureCurves;
vector<ConstraintPoint> constraintPoints;


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
	Shape* temp = new Shape(2);
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
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-0.3333f, 0.0f, 1.0f),
		glm::vec3(0.3333f, 0.0f, -1.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	};

	vector<glm::vec3> bezierPoints = bezier(ControlPoints);  // points for bezier curve
	vector<Shape> bezierLines; // lines for bezier curve
	
	Shape* line = new Shape(2);
	for (auto i = bezierPoints.begin(); i != bezierPoints.end() - 1; i++) {
		setLine(*line, *i, *(i + 1), returnColorRand2());
		bezierLines.push_back(*line);
	}
	delete(line);

	FeatureCurves.push_back(bezierLines);

	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡGenerate constraint points ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	ConstraintPoint c1; // constraint height
	c1.flag = ConstraintPoint::Flag::HAS_H;
	c1.h = 0.0f;
	c1.u = 0.0f;

	ConstraintPoint c2; // constraint left gradient
	c2.flag = ConstraintPoint::Flag::HAS_R | 
		ConstraintPoint::Flag::HAS_A |
		ConstraintPoint::Flag::HAS_ALPHA;
	c2.r = 0.05f;
	c2.b = 0.2f;
	c2.beta = 20.0f;
	c2.u = 0.5f;

	ConstraintPoint c3; // constraint both gradient
	c3.flag = ConstraintPoint::Flag::HAS_R |
		ConstraintPoint::Flag::HAS_A |
		ConstraintPoint::Flag::HAS_ALPHA |
		ConstraintPoint::Flag::HAS_B |
		ConstraintPoint::Flag::HAS_BETA;
	c3.r = 0.03f;
	c3.a = 0.2f;
	c3.alpha = 10.0f;
	c3.b = 0.1f;
	c3.beta = 25.0f;
	c3.u = 0.8f;

	ConstraintPoint c4; // constraint height
	c4.flag = ConstraintPoint::Flag::HAS_H;
	c4.h = 0.0f;
	c4.u = 1.0f;

	constraintPoints.push_back(c1);
	constraintPoints.push_back(c2);
	constraintPoints.push_back(c3);
	constraintPoints.push_back(c4);


	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡGenerate rectangles ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	// rectangles are generated on both sides of the line perpendicular to the tangent in each segment of the linearly approximated curve
	// since it is linearly approximated, the direction is perpendicular to each segment (u₂ - u₁)
	for (int i = 0; i < constraintPoints.size() - 1; i++) {
		float u1 = constraintPoints[i].u;
		float u2 = constraintPoints[i + 1].u;
		for (float u = u1; u < u2; u += 0.01f) {
		    glm::vec3 tangent = (pointOnBezier(ControlPoints, u + 0.01f) - pointOnBezier(ControlPoints, u)) / 0.01f;
			glm::vec3 normal = glm::normalize(glm::vec3(tangent.z, 0.0f, -tangent.x));

			// rectangles are generated in the direction of the normal and its opposite
			if (constraintPoints[i].flag & ConstraintPoint::Flag::HAS_R) {
			}
			float t = (u - u1) / (u2 - u1);
			float interpolatedr = lerp(constraintPoints[i].r, constraintPoints[i + 1].r, t);
			float interpolateda = lerp(constraintPoints[i].a, constraintPoints[i + 1].a, t);
			float interpolatedAlpha = lerp(constraintPoints[i].alpha, constraintPoints[i + 1].alpha, t);
			float interpolatedb = lerp(constraintPoints[i].b, constraintPoints[i + 1].b, t);
			float interpolatedBeta = lerp(constraintPoints[i].beta, constraintPoints[i + 1].beta, t);

			glm::vec3 rect[4];
			glm::vec3 p = pointOnBezier(ControlPoints, u);
			//p += normal * 


			// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ


			

			

			

		}

	}

	



	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

	glEnable(GL_DEPTH_TEST);
}

void idleScene() {
	glutPostRedisplay();
}

inline void draw(const vector<Shape> dia) {
	for (const auto& d : dia) {
		glBindVertexArray(d.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(d.TSR));
		if (d.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
		else if (d.vertices == 3) glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}
inline void drawWireframe(const vector<Shape> dia) {
	for (const auto& d : dia) {
		glBindVertexArray(d.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(d.TSR));
		if (d.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
		else if (d.vertices == 3) glDrawArrays(GL_LINE_LOOP, 0, 3);
	}
}
inline void draw(const Shape& dia) {
	glBindVertexArray(dia.VAO);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(dia.TSR));
	if (dia.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
	else if (dia.vertices == 3) glDrawArrays(GL_TRIANGLES, 0, 3);
}
inline void drawWireframe(const Shape& dia) {
	glBindVertexArray(dia.VAO);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(dia.TSR));
	if (dia.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
	else if (dia.vertices == 3) glDrawArrays(GL_LINE_LOOP, 0, 3);
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