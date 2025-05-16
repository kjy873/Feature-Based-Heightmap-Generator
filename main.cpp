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
#include <math.h>

#define RESOLUTION 1024

#define SEED 0

#define TERRAIN_SIZE 101

float PI = 3.14159265358979323846f;

using namespace std;

static random_device random;
static mt19937 gen(random());
static uniform_real_distribution<> distribution(0, 2.0 * PI);

bool MoveCameraForward = false;
bool MoveCameraBackward = false;
bool MoveCameraLeft = false;
bool MoveCameraRight = false;
float CameraYaw = -90.0f;
float CameraPitch = 0.0f;

float windowWidth = 800;
float windowHeight = 600;
const float defaultSize = 0.05;

constexpr float SAMPLE_INTERVAL = 0.1f;

inline float perlinSmooth(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

// one bezier curve by 4 control points
// sampling by 0.01
// this curve defines the directional flow of the terrain it represents
// During curve creation, the elevation component is initially set to zero, 
// and the actual terrain height is later diffused or interpolated based on the elevation constraints of the control points
vector<glm::vec3> bezier(glm::vec3 ControlPoints[4]) {

	glm::vec3 P = glm::vec3(0.0f, 0.0f, 0.0f);

	vector<glm::vec3> bezierPoints;

	for (float t = 0; t < 1; t += SAMPLE_INTERVAL) {
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
	v = (v / static_cast<float>(RESOLUTION)) * 2.0f - 1.0f;
}

inline float Cross2D(const glm::vec2& a, const glm::vec2& b) {
	return a.x * b.y - a.y * b.x;
}

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

bool isInTriangle(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
	float cross1 = Cross2D(b - a, p - a);
	float cross2 = Cross2D(c - b, p - b);
	float cross3 = Cross2D(a - c, p - c);
	return (cross1 >= 0 && cross2 >= 0 && cross3 >= 0) || (cross1 <= 0 && cross2 <= 0 && cross3 <= 0);

}

// linear interpolation
float lerp(float a, float b, float t) {
	return (1 - t) * a + t * b;
}

// hold interpolation
float hold(float p, float t) {
	return (1 - t) * p + t * p;
}

void Perlin(float noise[1024][1024]) {
	glm::vec2 gradient[1025][1025];
	for (int i = 0; i < 1025; i++) {
		for (int j = 0; j < 1025; j++) {
			float angle = distribution(gen);
			gradient[i][j] = glm::vec2(glm::cos(angle), glm::sin(angle));
		}
	}

	for (int y = 0; y < 1024; y++) {
		for (int x = 0; x < 1024; x++) {
			glm::vec2 input = glm::vec2(x, y);

			int x1 = floor(input.x);
			int y1 = floor(input.y);
			int x2 = x1 + 1;
			int y2 = y1 + 1;

			glm::vec2 d11 = input - glm::vec2(x1, y1);
			glm::vec2 d21 = input - glm::vec2(x2, y1);
			glm::vec2 d12 = input - glm::vec2(x1, y2);
			glm::vec2 d22 = input - glm::vec2(x2, y2);

			float dot11 = glm::dot(gradient[y1][x1], d11);
			float dot21 = glm::dot(gradient[y1][x2], d21);
			float dot12 = glm::dot(gradient[y2][x1], d12);
			float dot22 = glm::dot(gradient[y2][x2], d22);

			float u = perlinSmooth(input.x - x1);
			float v = perlinSmooth(input.y - y1);

			float interpolated1 = lerp(dot11, dot21, u);
			float interpolated2 = lerp(dot12, dot22, u);

			float result = lerp(interpolated1, interpolated2, v);

			noise[y][x] = result;

		}
	}
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
void mouseWheel(int button, int dir, int x, int y);
void init();
void initSplineSurface();
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
	vector<int> index;		
	vector<glm::vec3> normal;												// index for rectangle
	float u = 0.0f;
	float r = 0.0f;

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
			color.resize(8);
			index.resize(36);
			index = vector<int>{0, 2, 1, 0, 3, 2, 4, 6, 5, 4, 7, 6, 7, 5, 6, 7, 4, 5, 3, 6, 2, 3, 7, 6, 4, 3, 0, 4, 7, 3, 1, 6, 5, 1, 2, 6};
		}
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

void setRectangle(Shape& dst, const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3 vertex3, const glm::vec3 vertex4, const glm::vec3* c) {
	dst.position[0] = glm::vec3(vertex1);
	dst.position[1] = glm::vec3(vertex2);
	dst.position[2] = glm::vec3(vertex3);
	dst.position[3] = glm::vec3(vertex4);

	glm::vec3 v1 = vertex1 - vertex2;
	glm::vec3 v2 = vertex1 - vertex4;
	glm::vec3 n = glm::cross(v1, v2);

	dst.normal[0] = glm::cross(vertex3 - vertex1, vertex4 - vertex1);
	dst.normal[1] = glm::cross(vertex1 - vertex2, vertex3 - vertex2);
	dst.normal[2] = glm::cross(vertex2 - vertex3, vertex1 - vertex3);
	dst.normal[3] = glm::cross(vertex1 - vertex4, vertex3 - vertex4);

	for (int i = 0; i < 4; i++) {
		if (glm::length(dst.normal[i]) < 1e-6f) {
			dst.normal[0] = glm::vec3(0.0f, 1.0f, 0.0f);
			dst.normal[1] = glm::vec3(0.0f, 1.0f, 0.0f);
			dst.normal[2] = glm::vec3(0.0f, 1.0f, 0.0f);
			dst.normal[3] = glm::vec3(0.0f, 1.0f, 0.0f);
		}
		else {
			dst.normal[i] = glm::normalize(dst.normal[i]);
		}
	}
	

	for (int i = 0; i < 4; i++) {
		dst.color[i] = glm::vec3(c[i]);
	}
	
	InitBufferRectangle(dst.VAO, dst.position.data(), dst.position.size(), dst.color.data(), dst.color.size(), dst.index.data(), dst.index.size(), dst.normal.data(), dst.normal.size());
}

// 
void setHexahedron(vector<Shape>& dst, const glm::vec3 vertices[8], const glm::vec3* c) {
	
	Shape temp(4);

	glm::vec3 FrontColor[4] = { c[0], c[1], c[2], c[3] };
	glm::vec3 TopColor[4] = { c[4], c[5], c[1], c[0] };
	glm::vec3 BackColor[4] = { c[6], c[7], c[5], c[4] };
	glm::vec3 BottomColor[4] = { c[3], c[2], c[6], c[7] };
	glm::vec3 LeftColor[4] = { c[4], c[0], c[3], c[7] };
	glm::vec3 RightColor[4] = { c[1], c[5], c[6], c[2] };

	setRectangle(temp, vertices[0], vertices[1], vertices[2], vertices[3], FrontColor); // front
	dst.push_back(temp);
	setRectangle(temp, vertices[4], vertices[5], vertices[1], vertices[0], TopColor); // top
	dst.push_back(temp);
	setRectangle(temp, vertices[6], vertices[7], vertices[5], vertices[4], BackColor); // back
	dst.push_back(temp);
	setRectangle(temp, vertices[3], vertices[2], vertices[6], vertices[7], BottomColor); // bottom
	dst.push_back(temp);
	setRectangle(temp, vertices[4], vertices[0], vertices[3], vertices[7], LeftColor); // left
	dst.push_back(temp);
	setRectangle(temp, vertices[1], vertices[5], vertices[6], vertices[2], RightColor); // right
	dst.push_back(temp);
	
}

// normal vector of a, b
glm::vec3 getNormal(const glm::vec3& a, const glm::vec3& b) {
	return glm::normalize(glm::cross(a, b));
}

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;

unsigned int viewLocation;
unsigned int projectionLocation;
unsigned int modelTransformLococation;
vector <Shape> axes;
// camera
glm::vec3 camera[3];
glm::vec3 CameraForward;
glm::vec3 CameraRight;
glm::mat4 view = glm::mat4(1.0f);

vector<vector<Shape>> FeatureCurves;
vector<ConstraintPoint> constraintPoints;
vector<Shape> rectangles;						// render surface rectangles
vector<Shape> v_ControlPoints;				// render control points

glm::vec3 GridPoints[TERRAIN_SIZE][TERRAIN_SIZE];
vector<Shape> GridLines;
vector<Shape> GridRectangles;

vector<glm::vec3> SurfacePoints;

glm::vec3 LightSource = glm::vec3(0.0f, 5.0f, 0.0f);

float DiffusionGrid[TERRAIN_SIZE-1][TERRAIN_SIZE-1];

float noiseMap[1024][1024];

// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡGenerate rectangles ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
// rectangles are generated on both sides of the line perpendicular to the tangent in each segment of the linearly approximated curve
// since it is linearly approximated, the direction is perpendicular to each segment (u₂ - u₁)

// elevation is interpolated using cubic interpolation.
// since the four currently defined constraints do not allow elevation interpolation, it will be performed
// additionally, issues caused by curve intersections will also be addressed later
void Rasterization_rect(glm::vec3 ControlPoints[4], vector<ConstraintPoint>& constraintPoints, vector<Shape>& RectList) {
	for (int i = 0; i < constraintPoints.size() - 1; i++) {
		float u1 = constraintPoints[i].u;
		float u2 = constraintPoints[i + 1].u;

		// if this condition is satisfied, it means that an interpolated r value exists in the segment, so a rectangle is generated.
		// next, the rectangle is generated based on whether both constraintPoint A and B have r, or only one of them does.
		// then, the same branching logic is applied to a and b to modify the size of the rectangle.
		/*if (!(constraintPoints[i].flag & ConstraintPoint::Flag::HAS_H) && !(constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_H)) continue;
		if (!(constraintPoints[i].flag & ConstraintPoint::Flag::HAS_R) && !(constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_R)) continue;
		if (!(constraintPoints[i].flag & ConstraintPoint::Flag::HAS_A) && !(constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_A)) continue;
		if (!(constraintPoints[i].flag & ConstraintPoint::Flag::HAS_B) && !(constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_B)) continue;
		if (!(constraintPoints[i].flag & ConstraintPoint::Flag::HAS_BETA) && !(constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_BETA)) continue;
		if (!(constraintPoints[i].flag & ConstraintPoint::Flag::HAS_ALPHA) && !(constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_ALPHA)) continue;*/

		for (float u = u1; u < u2; u += SAMPLE_INTERVAL) {
			glm::vec3 tangent = (pointOnBezier(ControlPoints, u + SAMPLE_INTERVAL) - pointOnBezier(ControlPoints, u)) / SAMPLE_INTERVAL;
			glm::vec3 normal = glm::normalize(glm::vec3(tangent.z, 0.0f, -tangent.x));
			float t = (u - u1) / (u2 - u1);
			Shape rectA(4);
			Shape rectB(4);
			glm::vec3 black[4] = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) };


			setRectangle(rectA, pointOnBezier(ControlPoints, u) + normal * 0.0f,
				pointOnBezier(ControlPoints, u + SAMPLE_INTERVAL) + normal * 0.0f,
				pointOnBezier(ControlPoints, u + SAMPLE_INTERVAL),
				pointOnBezier(ControlPoints, u),
				black);

			setRectangle(rectB, pointOnBezier(ControlPoints, u),
				pointOnBezier(ControlPoints, u + SAMPLE_INTERVAL),
				pointOnBezier(ControlPoints, u + SAMPLE_INTERVAL) - normal * 0.0f,
				pointOnBezier(ControlPoints, u) - normal * 0.0f,
				black);

			//cout << rectA.color[0].x << ", " << rectA.color[0].y << ", " << rectA.color[0].z << endl;

			float interpolatedr = 0.0f;

			if (constraintPoints[i].flag & ConstraintPoint::Flag::HAS_R) {
				if (constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_R) {
					interpolatedr = lerp(constraintPoints[i].r, constraintPoints[i + 1].r, t);
				}
				else interpolatedr = hold(constraintPoints[i].r, t);
			}
			else interpolatedr = hold(constraintPoints[i + 1].r, t);


			setRectangle(rectA, pointOnBezier(ControlPoints, u) + normal * interpolatedr,
				pointOnBezier(ControlPoints, u + SAMPLE_INTERVAL) + normal * interpolatedr,
				pointOnBezier(ControlPoints, u + SAMPLE_INTERVAL),
				pointOnBezier(ControlPoints, u),
				black);

			setRectangle(rectB, pointOnBezier(ControlPoints, u),
				pointOnBezier(ControlPoints, u + SAMPLE_INTERVAL),
				pointOnBezier(ControlPoints, u + SAMPLE_INTERVAL) - normal * interpolatedr,
				pointOnBezier(ControlPoints, u) - normal * interpolatedr,
				black);


			if ((constraintPoints[i].flag & ConstraintPoint::Flag::HAS_A) || (constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_A)) {
				float interpolateda = 0.0f;
				float interpolatedAlpha = 0.0f;
				if (constraintPoints[i].flag & ConstraintPoint::Flag::HAS_A) {
					if (constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_A) {
						interpolateda = lerp(constraintPoints[i].a, constraintPoints[i + 1].a, t);
						interpolatedAlpha = lerp(constraintPoints[i].alpha, constraintPoints[i + 1].alpha, t);
					}
					else {
						interpolateda = hold(constraintPoints[i].a, t);
						interpolatedAlpha = hold(constraintPoints[i].alpha, t);
					}
				}
				else {
					interpolateda = hold(constraintPoints[i + 1].a, t);
					interpolatedAlpha = hold(constraintPoints[i + 1].alpha, t);
				}

				interpolatedAlpha = glm::clamp(interpolatedAlpha, -30.0f, 30.0f);
				float tan = glm::tan(glm::radians(interpolatedAlpha));

				glm::vec3 gradientVector = glm::vec3(normal.x, normal.z, tan);
				gradientVector = glm::normalize(gradientVector);

				glm::vec3 clampColor = glm::vec3(glm::clamp(gradientVector.x, 0.0f, 1.0f),
					glm::clamp(gradientVector.y, 0.0f, 1.0f),
					glm::clamp(gradientVector.z, 0.0f, 1.0f));
				//cout << "clampColor: " << clampColor.x << ", " << clampColor.y << ", " << clampColor.z << endl;
				//cout << gradientVector.x << ", " << gradientVector.y << ", " << gradientVector.z << endl;
				glm::vec3 colorA[4] = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), clampColor, clampColor };
				setRectangle(rectA, (rectA).position[0] + normal * interpolateda,
					(rectA).position[1] + normal * interpolateda,
					(rectA).position[2],
					(rectA).position[3],
					colorA);
				//cout << "rectA.color[0]: " << rectA.color[0].x << ", " << rectA.color[0].y << ", " << rectA.color[0].z << endl;
			}


			if ((constraintPoints[i].flag & ConstraintPoint::Flag::HAS_B) || (constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_B)) {
				float interpolatedb = 0.0f;
				float interpolatedBeta = 0.0f;
				if (constraintPoints[i].flag & ConstraintPoint::Flag::HAS_B) {
					if (constraintPoints[i + 1].flag & ConstraintPoint::Flag::HAS_B) {
						interpolatedb = lerp(constraintPoints[i].b, constraintPoints[i + 1].b, t);
						interpolatedBeta = lerp(constraintPoints[i].beta, constraintPoints[i + 1].beta, t);
					}
					else {
						interpolatedb = hold(constraintPoints[i].b, t);
						interpolatedBeta = hold(constraintPoints[i].beta, t);
					}
				}
				else {
					interpolatedb = hold(constraintPoints[i + 1].b, t);
					interpolatedBeta = hold(constraintPoints[i + 1].beta, t);
				}

				interpolatedBeta = glm::clamp(interpolatedBeta, -30.0f, 30.0f);

				float tan = glm::tan(glm::radians(interpolatedBeta));
				glm::vec3 gradientVector = glm::vec3(-normal.x, -normal.z, tan);

				gradientVector = glm::normalize(gradientVector);

				glm::vec3 clampColor = glm::vec3(glm::clamp(gradientVector.x, 0.0f, 1.0f),
					glm::clamp(gradientVector.y, 0.0f, 1.0f),
					glm::clamp(gradientVector.z, 0.0f, 1.0f));
				glm::vec3 colorB[4] = { clampColor, clampColor, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) };


				setRectangle(rectB, (rectB).position[0],
					(rectB).position[1],
					(rectB).position[2] - normal * interpolatedb,
					(rectB).position[3] - normal * interpolatedb,
					colorB);

				//cout << "rectB.color[0]: " << rectB.color[0].x << ", " << rectB.color[0].y << ", " << rectB.color[0].z << endl;
			}

			rectA.u = u;
			rectB.u = u;
			rectA.r = interpolatedr;
			rectB.r = interpolatedr;
			RectList.push_back(rectA);
			RectList.push_back(rectB);




			// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

		}

	}
}

// function to generate b-spline surface
float BasisFunction(int index, int degree, float t, vector<float> KnotVector) {
	if (degree == 0) {
		if (t >= KnotVector[index] && t < KnotVector[index + 1])return 1.0f;
		else return 0.0f;
	}

	float left{0.0f};
	float right{0.0f};

	if (KnotVector[index + degree] != KnotVector[index]) {
		left = (t - KnotVector[index]) / (KnotVector[index + degree] - KnotVector[index]) * BasisFunction(index, degree - 1, t, KnotVector);
	}
	if(KnotVector[index + degree + 1] != KnotVector[index + 1]) {
		right = (KnotVector[index + degree + 1] - t) / (KnotVector[index + degree + 1] - KnotVector[index + 1]) * BasisFunction(index + 1, degree - 1, t, KnotVector);
	}

	if (t == KnotVector.back() && index == KnotVector.size() - degree - 2)
		return 1.0f;

	return left + right;

}

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
	initSplineSurface();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyboardUp);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(10, CameraTimer, 0);
	glutMouseWheelFunc(mouseWheel);

	glutMainLoop();
}

void init() {
	glClearColor(backgroundColor.R, backgroundColor.G, backgroundColor.B, backgroundColor.A);

	viewLocation = glGetUniformLocation(shaderProgramID, "viewTransform");
	projectionLocation = glGetUniformLocation(shaderProgramID, "projectionTransform");
	modelTransformLococation = glGetUniformLocation(shaderProgramID, "modelTransform");
	
	unsigned int lightSourceLocation = glGetUniformLocation(shaderProgramID, "lightPos");

	// axes
	Shape* temp = new Shape(2);
	setLine(*temp, glm::vec3(-4.0, 0.0, 0.0), glm::vec3(4.0, 0.0, 0.0), returnColorRand2());
	axes.push_back(*temp);
	setLine(*temp, glm::vec3(0.0, -4.0, 0.0), glm::vec3(0.0, 4.0, 0.0), returnColorRand2());
	axes.push_back(*temp);
	setLine(*temp, glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 0.0, 4.0), returnColorRand2());
	axes.push_back(*temp);
	delete(temp);

	camera[0] = glm::vec3(0.4, 0.2, 0.8);
	camera[1] = glm::vec3(0.5, 0.0, 0.5);
	camera[2] = glm::vec3(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(camera[0], camera[1], camera[2]);
	CameraForward = camera[1] - camera[0];

	glEnable(GL_DEPTH_TEST);
}

void initSplineSurface() {
	glm::vec3 controlPoints[4][4] = {
	{ {0, 0, 0}, {1, 1, 0}, {2, 1, 0}, {3, 1, 0} },
	{ {0, 0, 1}, {1, 0, 1}, {2, 0, 1}, {3, 0, 1} },
	{ {0, 1, 2}, {1, 1, 2}, {2, 2, 2}, {3, 1, 2} },
	{ {0, 0, 3}, {1, 0, 3}, {2, 0, 3}, {3, 1, 3} }
	};

	int ControlPointRows = 4;
	int ControlPointCols = 4;

	int u_degree = 3;
	int v_degree = 3;

	vector<float> KnotVectorU = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	vector<float> KnotVectorV = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	int sampleCount = (int)(1.0f / SAMPLE_INTERVAL) + 1;

	for (int p = 0; p < sampleCount; p++) {
		float u = p * SAMPLE_INTERVAL;
		for (int q = 0; q < sampleCount; q++) {
			float v = q * SAMPLE_INTERVAL;
			glm::vec3 CurrentPoint(0.0f, 0.0f, 0.0f);
			for (int i = 0; i < ControlPointRows; i++) {
				float PointU = BasisFunction(i, u_degree, u, KnotVectorU);
				for (int j = 0; j < ControlPointCols; j++) {
					float PointV = BasisFunction(j, v_degree, v, KnotVectorV);
					CurrentPoint += controlPoints[i][j] * PointU * PointV;
				}
			}
			SurfacePoints.push_back(CurrentPoint);
		}
	}

	int SizeU = sqrt(SurfacePoints.size());
	int SizeV = sqrt(SurfacePoints.size());

	

	Shape* tempRect = new Shape(4);
	for (int i = 0; i < SizeU - 1; i++) {
		for (int j = 0; j < SizeV - 1; j++) {
			int index = i * SizeV + j;
			glm::vec3 p1 = SurfacePoints[index];
			glm::vec3 p2 = SurfacePoints[index + 1];
			glm::vec3 p3 = SurfacePoints[index + SizeV + 1];
			glm::vec3 p4 = SurfacePoints[index + SizeV];
			glm::vec3 gray[4] = { glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f) };
			setRectangle(*tempRect, p1, p2, p3, p4, gray);
			rectangles.push_back(*tempRect);
		}
	}
	cout << rectangles.size() << endl;

	float half = 0.01f;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			glm::vec3 points[8] = { 
				controlPoints[i][j] + glm::vec3(-half, half, +half),
				controlPoints[i][j] + glm::vec3(+half, +half, +half),
				controlPoints[i][j] + glm::vec3(+half, -half, +half),
				controlPoints[i][j] + glm::vec3(-half, -half, +half),
				controlPoints[i][j] + glm::vec3(-half, +half, -half),
				controlPoints[i][j] + glm::vec3(+half, +half, -half),
				controlPoints[i][j] + glm::vec3(+half, -half, -half),
				controlPoints[i][j] + glm::vec3(-half, -half, -half) 
			};

			glm::vec3 gray[8] = { glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
				glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f) };
			setHexahedron(v_ControlPoints, points, gray);
		}
	}

}

void idleScene() {
	glutPostRedisplay();
}

inline void draw(const vector<Shape>& dia) {
	GLuint currentVAO = -1;
	for (const auto& d : dia) {
		if (d.VAO != currentVAO) {
			glBindVertexArray(d.VAO);
			currentVAO = d.VAO;
		}
		glUniformMatrix4fv(modelTransformLococation, 1, GL_FALSE, glm::value_ptr(d.TSR));
		switch (d.vertices) {
		case 2: glDrawArrays(GL_LINES, 0, 2); break;
		case 3: glDrawArrays(GL_TRIANGLES, 0, 3); break;
		case 4: glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); break;
		}
	}
}
inline void drawWireframe(const vector<Shape> dia) {
	for (const auto& d : dia) {
		glBindVertexArray(d.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(d.TSR));
		if (d.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
		else if (d.vertices == 3) glDrawArrays(GL_LINE_LOOP, 0, 3);
		else if (d.vertices == 4) glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0);
	}
}
inline void draw(const Shape& dia) {
	glBindVertexArray(dia.VAO);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(dia.TSR));
	if (dia.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
	else if (dia.vertices == 3) glDrawArrays(GL_TRIANGLES, 0, 3);
	else if (dia.vertices == 4) glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
inline void drawWireframe(const Shape& dia) {
	glBindVertexArray(dia.VAO);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(dia.TSR));
	if (dia.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
	else if (dia.vertices == 3) glDrawArrays(GL_LINE_LOOP, 0, 3);
	else if (dia.vertices == 4) glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0);
}

GLvoid drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw(axes);
	
	/*for (const auto& c : FeatureCurves) {
		draw(c);
	}*/

	draw(rectangles);
	//draw(GridLines);
	//draw(GridRectangles);

	draw(v_ControlPoints);

	view = glm::lookAt(camera[0], camera[0] + CameraForward, camera[2]);

	
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
	projection = glm::translate(projection, glm::vec3(0.0, 0.0, -2.0));
	
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

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

//void pickObject();

void mouse(int button, int state, int x, int y) {
	mgl = transformMouseToGL(x, y);
	
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		glutMotionFunc(motion);
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		glutMotionFunc(NULL);
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		glutMotionFunc(NULL);
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		glutMotionFunc(NULL);
	}

	preMousePosition = mgl;
	glutPostRedisplay();
}

void mouseWheel(int wheel, int direction, int x, int y) {
	if (direction > 0) {
		camera[0] += CameraForward * 0.1f;
	}
	else {
		camera[0] -= CameraForward * 0.1f;
	}
	glutPostRedisplay();
}

void motion(int x, int y) {
	cout << "mouse motion" << endl;
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

	glutPostRedisplay();

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
void InitBufferTriangle(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize) {
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
void InitBufferRectangle(GLuint& VAO, const glm::vec3* position, const int positionSize,
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
