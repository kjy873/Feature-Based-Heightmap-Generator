#pragma once

#define _CRT_SECURE_NO_WARININGS

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "ImGuizmo.h"

#include "Mesh.h"

#include <glew.h>

#include <glm.hpp>
#include <ext.hpp>
#include <gtc/matrix_transform.hpp>

#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <random>
#include <algorithm>
#include <sstream>
#include <string.h>
#include <string>
#include <math.h>
#include <chrono>
#include <functional>

using namespace std;
using namespace glm;

#define RESOLUTION 1024

#define SEED 0

#define TERRAIN_SIZE 101

GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

struct NoiseParameters {
	float frequency = 32.0f;
	int octaves = 4;
	float persistence = 0.5f;
	float lacunarity = 2.0f;
	string noiseType = "Perlin";
};


enum class ToolType {
	none,
	confirm
};

// Constraint point
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

// shape struct
struct Shape {
	GLuint VAO{ NULL };												// VAO
	vector<glm::vec3> position;										// vertex positions
	vector<glm::vec3> currentPosition;								// current vertex positions
	vector<glm::vec3> color;										// vertex colors
	int vertices = 0;												// number of vertices
	glm::mat4 TSR = glm::mat4(1.0f);								// transform matrix
	vector<ConstraintPoint> CP;										// constraint points

	// 시각화된 제어점(hexahedron)에만 사용되는 변수들
	glm::vec3* linkedPosition = nullptr;
	int linkedRows = 0;											
	int linkedCols = 0;					
	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	vector<int> index;
	vector<glm::vec3> normal;												// index for rectangle
	float u = 0.0f;
	float r = 0.0f;

	bool IsLine = false;

	//2D shape
	Shape(int vertexCount) : vertices(vertexCount) {
		position.resize(vertices);
		currentPosition.resize(vertices);
		color.resize(vertices);
		normal.resize(vertices);

		for (int i = 0; i < vertices; i++) {
			position[i] = glm::vec3(0.0f, 0.0f, 0.0f);
			currentPosition[i] = glm::vec3(0.0f, 0.0f, 0.0f);
			color[i] = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		if (vertices == 4) {
			index.resize(6);
			index = vector<int>{ 0, 2, 1, 0, 3, 2 };
		}

		if (vertices == 8) {
			position.resize(8);
			color.resize(8);
			normal.resize(8);
			index.resize(36);
			index = vector<int>{// front
								0, 1, 2,  2, 3, 0,
								// right
								1, 5, 6,  6, 2, 1,
								// back
								5, 4, 7,  7, 6, 5,
								// left
								4, 0, 3,  3, 7, 4,
								// top
								4, 5, 1,  1, 0, 4,
								// bottom
								3, 2, 6,  6, 7, 3 };
		}
	}
};

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
glm::vec3* returnColorBK2() {
	glm::vec3 color[2];
	for (int i = 0; i < 2; i++) color[i] = glm::vec3(0, 0, 0);
	return color;
}
glm::vec3* returnColorRD2() {
	glm::vec3 color[2];
	for (int i = 0; i < 2; i++) color[i] = glm::vec3(255, 0, 0);
	return color;
}
glm::vec3* returnColorRand4() {
	glm::vec3 color[4];
	for (int i = 0; i < 4; i++) color[i] = glm::vec3((float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255);
	return color;
}
glm::vec3* setColor4(const glm::vec3& c1, const glm::vec3& c2, const glm::vec3& c3, const glm::vec3& c4) {
	glm::vec3 color[4];
	color[0] = glm::vec3(c1);
	color[1] = glm::vec3(c2);
	color[2] = glm::vec3(c3);
	color[3] = glm::vec3(c4);
	return color;
}

// mouse point to GL coordinate
struct mouseCoordGL {
	double x;
	double y;
};
mouseCoordGL transformMouseToGL(double x, double y, double windowWidth, double windowHeight) {
	mouseCoordGL m;
	m.x = (2.0f * x) / windowWidth - 1.0f;
	m.y = 1.0f - (2.0f * y) / windowHeight;
	return m;
}

void InitBufferLine(GLuint& shaderProgramID, GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize) {
	//cout << "버퍼 초기화" << endl;
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
void InitBufferTriangle(GLuint& shaderProgramID, GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize) {
	//cout << "버퍼 초기화" << endl;
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
void InitBufferRectangle(GLuint& shaderProgramID, GLuint& VAO, const glm::vec3* position, const int positionSize,
	const glm::vec3* color, const int colorSize,
	const int* index, const int indexSize, const glm::vec3* normals, const int normalSize) {
	//cout << "EBO 초기화" << endl;
	GLuint VBO_position, VBO_color, VBO_normal, EBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glBufferData(GL_ARRAY_BUFFER, 3 * positionSize * sizeof(float), position, GL_STATIC_DRAW);

	GLint pAttribute = glGetAttribLocation(shaderProgramID, "vPos");
	if (pAttribute < 0) cout << "pAttribute < 0" << endl;
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	while (pAttribute < 0);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
	glBufferData(GL_ARRAY_BUFFER, 3 * colorSize * sizeof(float), color, GL_STATIC_DRAW);

	GLint cAttribute = glGetAttribLocation(shaderProgramID, "vColor");
	if (cAttribute < 0) cout << "cAttribute < 0" << endl;
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);

	glGenBuffers(1, &VBO_normal);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_normal);
	glBufferData(GL_ARRAY_BUFFER, 3 * normalSize * sizeof(float), normals, GL_STATIC_DRAW);

	GLint nAttribute = glGetAttribLocation(shaderProgramID, "vNormal");
	if (nAttribute < 0) {
		cout << "nAttribute < 0" << endl;
		cout << normalSize << endl;
		for (int i = 0; i < normalSize; i++) {
			cout << normals[i].x << ", " << normals[i].y << ", " << normals[i].z << endl;
		}
	}
	glVertexAttribPointer(nAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(nAttribute);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(int), index, GL_STATIC_DRAW);

}

bool intersectRayTriangle(const glm::vec3& rayBegin, const glm::vec3& rayEnd,
	const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
	glm::vec3& intersectionPoint, float& distance) {

	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;

	glm::vec3 h = glm::cross(rayEnd - rayBegin, edge2);

	float a = glm::dot(edge1, h);
	if (a > -0.00001f && a < 0.00001f) return false;

	float f = 1.0f / a;

	glm::vec3 s = rayBegin - v0;

	float u = f * glm::dot(s, h);
	if (u < 0.0f || u > 1.0f) return false;

	glm::vec3 q = glm::cross(s, edge1);
	float v = f * glm::dot(rayEnd - rayBegin, q);
	if (v < 0.0f || u + v > 1.0f) return false;

	float t = f * glm::dot(edge2, q);
	if (t < 0.0f) return false;

	distance = t;
	intersectionPoint = rayBegin + (rayEnd - rayBegin) * t;
	return true;

}

inline bool intersertRayRectangle(const glm::vec3& rayBegin, const glm::vec3& rayEnd,
	const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
	glm::vec3& intersectionPoint, float& distance) {

	float t1{ 0.0f }, t2{ 0.0f };
	glm::vec3 p1{ 0.0f, 0.0f, 0.0f }, p2{ 0.0f, 0.0f, 0.0f };

	if (intersectRayTriangle(rayBegin, rayEnd, v0, v1, v2, p1, t1)) {
		distance = t1;
		intersectionPoint = p1;
		return true;
	}
	if (intersectRayTriangle(rayBegin, rayEnd, v0, v2, v3, p2, t2)) {
		distance = t2;
		intersectionPoint = p2;
		return true;
	}
	return false;
}

inline bool intersectRayRectangleShape(const Shape& rectangle, const glm::vec3& rayBegin, const glm::vec3& rayEnd,
	glm::vec3& intersectionPoint, float& distance) {
	return intersertRayRectangle(rayBegin, rayEnd, rectangle.position[0], rectangle.position[1],
		rectangle.position[2], rectangle.position[3], intersectionPoint, distance);
}

inline bool intersectRayHexahedron(const HexahedronMesh& Hexahedron, const glm::vec3& rayBegin, const glm::vec3& rayEnd,
	glm::vec3& intersectionPoint, float& distance, int& intersectedIndex) {
	bool intersected = false;
	float minDistance = FLT_MAX;

	for (int i = 0; i < Hexahedron.GetIndex().size(); i += 3) {
		glm::vec3 v0 = glm::vec3(Hexahedron.TSR * glm::vec4(Hexahedron.GetPosition()[Hexahedron.GetIndex()[i]], 1.0f));
		glm::vec3 v1 = glm::vec3(Hexahedron.TSR * glm::vec4(Hexahedron.GetPosition()[Hexahedron.GetIndex()[i + 1]], 1.0f));
		glm::vec3 v2 = glm::vec3(Hexahedron.TSR * glm::vec4(Hexahedron.GetPosition()[Hexahedron.GetIndex()[i + 2]], 1.0f));

		glm::vec3 p;
		float t;

		if (intersectRayTriangle(rayBegin, rayEnd, v0, v1, v2, p, t)) {
			if (t < minDistance) {
				minDistance = t;
				intersectionPoint = p;
				distance = t;
				intersectedIndex = i / 3;
				intersected = true;
			}
		}
	}

	return intersected;
}

void normalize(glm::vec3& v) {
	v = (v / static_cast<float>(RESOLUTION)) * 2.0f - 1.0f;
}

inline float Cross2D(const glm::vec2& a, const glm::vec2& b) {
	return a.x * b.y - a.y * b.x;
}

bool isInTriangle(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
	float cross1 = Cross2D(b - a, p - a);
	float cross2 = Cross2D(c - b, p - b);
	float cross3 = Cross2D(a - c, p - c);
	return (cross1 >= 0 && cross2 >= 0 && cross3 >= 0) || (cross1 <= 0 && cross2 <= 0 && cross3 <= 0);
}

// linear interpolation
float Lerp(float a, float b, float t) {
	return (1 - t) * a + t * b;
}
// hold interpolation
float hold(float p, float t) {
	return (1 - t) * p + t * p;
}

void init();
GLvoid drawScene();
inline void draw(const vector<LineMesh>& Mesh);
inline void draw(const vector<Shape>& dia);
inline void drawWireframe(GLuint &ShaderProgramID, const vector<Shape> dia);
inline void draw(GLuint& ShaderProgramID, const Shape& dia);
inline void drawWireframe(GLuint& ShaderProgramID, const Shape& dia);
float BasisFunction(int index, int degree, float t, vector<float> KnotVector);
bool intersectRayTriangle(const glm::vec3& rayBegin, const glm::vec3& rayEnd,
	const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
	glm::vec3& intersectionPoint, float& distance);
inline bool intersertRayRectangle(const glm::vec3& rayBegin, const glm::vec3& rayEnd,
	const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
	glm::vec3& intersectionPoint, float& distance);
inline bool intersectRayRectangleShape(const Shape& rectangle, const glm::vec3& rayBegin, const glm::vec3& rayEnd,
	glm::vec3& intersectionPoint, float& distance);
inline bool intersectRayHexahedron(const HexahedronMesh& Hexahedron, const glm::vec3& rayBegin, const glm::vec3& rayEnd,
	glm::vec3& intersectionPoint, float& distance, int& intersectedIndex);
vector<float> initKnotVector(int n, int degree);
glm::vec3 getNormal(const glm::vec3& a, const glm::vec3& b);
glm::vec3 RayfromMouse(mouseCoordGL mgl, const glm::mat4& ProjectionMatrix, const glm::mat4& view);
bool isInTriangle(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);
float Lerp(float a, float b, float t);
float hold(float p, float t);
inline float perlinSmooth(float t);
float Perlin(const glm::vec2& input, int seed);
float SimplexAttenuation(const glm::vec2& point, const glm::vec2& gradient);
float Simplex(const glm::vec2& input, int seed);
float NoiseCombiner1(const glm::vec2& p, const float& width, const float& height, const int& seed,
	float frequency, int octaves, float persistence, float lacunarity, const string& noiseType);
std::function<float(const glm::vec2&)> NoiseSelector(const float& width, const float& height, const int& seed,
	float frequency, int octaves, float persistence, float lacunarity, const string& noiseType);
vector<glm::vec3> bezier(glm::vec3 ControlPoints[4]);
glm::vec3 pointOnBezier(glm::vec3 ControlPoints[4], float u);
void normalize(glm::vec3& v);
inline float Cross2D(const glm::vec2& a, const glm::vec2& b);
void initSplineSurface(vector<vector<glm::vec3>>& ControlPoints, const int& nRows, const int& nCols);
GLvoid Keyboard(GLFWwindow* window);
void CallbackMouseWheel(GLFWwindow* window, double xoffset, double yoffset);

void Rasterization_rect(glm::vec3 ControlPoints[4], vector<ConstraintPoint>& constraintPoints, vector<Shape>& RectList);

void CallbackMouseButton(GLFWwindow* window, int button, int action, int mods);
void MouseMoveRightButton(GLFWwindow* window);
void CallbackMouseMove(GLFWwindow* window, double xpos, double ypos);

void DrawPanel();

void UpdateToolInteraction();

vector<vector<glm::vec3>> MakeInitialControlPoints(const int& Rows, const int& Cols);

void ExportHeightMap(const char* FileName);