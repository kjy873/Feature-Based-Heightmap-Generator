#include "base.h"

void setLine(Shape& dst, const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3* c) {
	for (int i = 0; i < 2; i++) {
		dst.color[i] = glm::vec3(c[i]);
	}
	dst.position[0] = glm::vec3(vertex1);
	dst.position[1] = glm::vec3(vertex2);

	InitBufferLine(shaderProgramID, dst.VAO, dst.position.data(), dst.position.size(), dst.color.data(), dst.color.size());
}
void setRectangle(Shape& dst, const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3 vertex3, const glm::vec3 vertex4, const glm::vec3* c) {
	dst.position[0] = glm::vec3(vertex1);
	dst.position[1] = glm::vec3(vertex2);
	dst.position[2] = glm::vec3(vertex3);
	dst.position[3] = glm::vec3(vertex4);

	glm::vec3 normal = glm::normalize(glm::cross(vertex2 - vertex1, vertex4 - vertex1));
	for (int i = 0; i < 4; i++) {
		dst.normal[i] = normal;
	}

	for (int i = 0; i < 4; i++) {
		dst.color[i] = glm::vec3(c[i]);
	}

	InitBufferRectangle(shaderProgramID, dst.VAO, dst.position.data(), dst.position.size(), dst.color.data(), dst.color.size(), dst.index.data(), dst.index.size(), dst.normal.data(), dst.normal.size());
}
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
void setHexahedron(Shape& dst, const glm::vec3 center, float half, const glm::vec3& c) {
	
	glm::vec3 vertices[8] = {
	   center + glm::vec3(-half,  half,  half),
	   center + glm::vec3(half,  half,  half),
	   center + glm::vec3(half, -half,  half),
	   center + glm::vec3(-half, -half,  half),
	   center + glm::vec3(-half,  half, -half),
	   center + glm::vec3(half,  half, -half),
	   center + glm::vec3(half, -half, -half),
	   center + glm::vec3(-half, -half, -half)
	};

	for (int i = 0; i < 8; ++i) {
		dst.position[i] = vertices[i];
		dst.color[i] = c;
	}

	vector<glm::vec3> normalSum(8, glm::vec3(0.0f));
	vector<int> normalCount(8, 0);
	for (int i = 0; i < 36; i += 3) {
		int i0 = dst.index[i];
		int i1 = dst.index[i + 1];
		int i2 = dst.index[i + 2];

		glm::vec3& p0 = dst.position[i0];
		glm::vec3& p1 = dst.position[i1];
		glm::vec3& p2 = dst.position[i2];

		glm::vec3 faceNormal = glm::normalize(glm::cross(p1 - p0, p2 - p0));

		normalSum[i0] += faceNormal;
		normalSum[i1] += faceNormal;
		normalSum[i2] += faceNormal;

		normalCount[i0]++;
		normalCount[i1]++;
		normalCount[i2]++;
	}

	for (int i = 0; i < 8; ++i) {
		if (normalCount[i] > 0)
			dst.normal[i] = glm::normalize(normalSum[i] / (float)normalCount[i]);
		else
			dst.normal[i] = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	InitBufferRectangle(shaderProgramID, dst.VAO, dst.position.data(), dst.position.size(), 
						dst.color.data(), dst.color.size(), 
						dst.index.data(), dst.index.size(), dst.normal.data(), dst.normal.size());
}

void setSurface(Shape& dst, const vector<glm::vec3>& vertices, const vector<int>& indices, const vector<glm::vec3>& normals, const glm::vec3& c) {
	dst.position = vertices;
	dst.index = indices;
	dst.normal = normals;
	dst.color.resize(vertices.size());

	for(int i = 0; i < vertices.size(); i++) {
		dst.color[i] = c;
	}

	InitBufferRectangle(shaderProgramID, dst.VAO, dst.position.data(), dst.position.size(), 
						dst.color.data(), dst.color.size(), 
		dst.index.data(), dst.index.size(), dst.normal.data(), dst.normal.size());
}

float PI = 3.14159265358979323846f;

static random_device random;
static mt19937 gen(random());
static uniform_real_distribution<> distribution(0, 2.0 * PI);

bool MoveCameraForward = false;
bool MoveCameraBackward = false;
bool MoveCameraLeft = false;
bool MoveCameraRight = false;
float CameraYaw = -90.0f;
float CameraPitch = 0.0f;

bool MouseRightButtonPressed = false;
double LastMouseX = 0.0;
double LastMouseY = 0.0;

bool ControlPointRender = true;

double windowWidth = 1920;
double windowHeight = 1080;
int FrameBufferWidth;
int FrameBufferHeight;
const float defaultSize = 0.05;

float CameraSpeed = 0.01f;

float SAMPLE_INTERVAL = 0.05f;

COLOR backgroundColor{ 1.0f, 1.0f, 1.0f, 0.0f };

mouseCoordGL mgl;
mouseCoordGL preMousePosition;

GLint width, height;

unsigned int viewLocation;
unsigned int projectionLocation;
unsigned int modelTransformLococation;

vector <Shape> axes;
// camera
glm::vec3 camera[3];
glm::vec3 CameraForward;
glm::vec3 CameraRight;
glm::mat4 view = glm::mat4(1.0f);

glm::mat4 projection = glm::mat4(1.0f);

vector<vector<glm::vec3>> controlPoints;
vector<vector<glm::vec3>> controlPoints_modifier;
vector<vector<glm::vec3>> controlPoints_initial;
vector<Shape> ControlLines;
vector<vector<Shape>> FeatureCurves;
vector<ConstraintPoint> constraintPoints;
vector<Shape> rectangles;						// render surface rectangles
vector<Shape> v_ControlPoints;				// render control points
Shape* PickedControlPoint{ NULL };	// picked control point

glm::vec3 GridPoints[TERRAIN_SIZE][TERRAIN_SIZE];
vector<Shape> GridLines;
vector<Shape> GridRectangles;

vector<glm::vec3> SurfacePoints;

glm::vec3 LightSource = glm::vec3(5.0f, 5.0f, 5.0f);

float DiffusionGrid[TERRAIN_SIZE - 1][TERRAIN_SIZE - 1];

float noiseMap[1024][1024];

ToolType CurrentTool = ToolType::none;

glm::mat4 PickedObjectModelTransform = glm::mat4(1.0f);

glm::vec3 PickedObjectPos = glm::vec3(0.0f, 0.0f, 0.0f);

Shape Surface(0);


int main() {

	cout << "main" << endl;
    // GLFW 초기화
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // (MacOS의 경우 추가 필요)
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // 윈도우 생성
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "FBHG", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

	// 콜백 함수 등록
	glfwSetMouseButtonCallback(window, CallbackMouseButton);

    // OpenGL context를 현재 쓰레드로 지정
    glfwMakeContextCurrent(window);

    // GLEW 초기화
    glewExperimental = GL_TRUE; // 최신 기능을 사용할 수 있도록 설정
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    // 뷰포트 설정
    glViewport(0, 0, windowWidth, windowHeight);

	InitShader(shaderProgramID, vertexShader, "vertex.glsl", fragmentShader, "fragment.glsl");
	glUseProgram(shaderProgramID);

	glfwGetFramebufferSize(window, &FrameBufferWidth, &FrameBufferHeight);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontFromFileTTF("fonts/NotoSansKR-Regular.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesKorean());
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");


	init();
	controlPoints.resize(4, vector<glm::vec3>(4));
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			controlPoints[i][j] = glm::vec3(j, 0.0f, i);
		}
	}
	controlPoints_modifier = controlPoints;
	initSplineSurface(controlPoints, 4, 4);

	glfwSetScrollCallback(window, CallbackMouseWheel);
    // 루프
    while (!glfwWindowShouldClose(window)) {
        // 입력 처리 (예: ESC 키)
		Keyboard(window);
		if(MouseRightButtonPressed)MouseMoveRightButton(window);
        // 렌더링
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		drawScene();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();


		// imguizmo set, call
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
		ImGuizmo::SetRect(0, 0, FrameBufferWidth, FrameBufferHeight);
		if (PickedControlPoint)
		{
			ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
				ImGuizmo::TRANSLATE, ImGuizmo::WORLD, glm::value_ptr(PickedObjectModelTransform));

			//ImGuizmo::DecomposeMatrixToComponents() 변환을 값으로 바꿈


			PickedControlPoint->TSR = PickedObjectModelTransform; // ImGuizmo가 사용 중이면 PickedControlPoint의 변환을 업데이트
			glm::vec3 newPosition = glm::vec3(PickedObjectModelTransform[3]);
			controlPoints_modifier[PickedControlPoint->linkedRows][PickedControlPoint->linkedCols] = newPosition;
			PickedObjectPos = newPosition;
		}

		DrawPanel();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



        // 버퍼 교환
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

    // 종료 처리
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
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
	CameraRight = glm::normalize(glm::cross(CameraForward, camera[2]));


	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);

	glEnable(GL_DEPTH_TEST);
}

GLvoid drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw(axes);
	
	//draw(rectangles);

	draw(Surface);

	if (ControlPointRender) {
		glDisable(GL_DEPTH_TEST);
		draw(v_ControlPoints);
		draw(ControlLines);
		glEnable(GL_DEPTH_TEST);
	}
	view = glm::lookAt(camera[0], camera[0] + CameraForward, camera[2]);

	float aspect = static_cast<float>(windowWidth) / windowHeight;

	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);
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
		case 8: glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); break;
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
	else if (dia.position.size() > 4) glDrawElements(GL_TRIANGLES, dia.index.size(), GL_UNSIGNED_INT, 0);
}
inline void drawWireframe(const Shape& dia) {
	glBindVertexArray(dia.VAO);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(dia.TSR));
	if (dia.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
	else if (dia.vertices == 3) glDrawArrays(GL_LINE_LOOP, 0, 3);
	else if (dia.vertices == 4) glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0);
}

//inline void DrawSurface(const Shape& surface) {
//	glBindVertexArray(surface.VAO);
//	glUniformMatrix4fv(modelTransformLococation, 1, GL_FALSE, glm::value_ptr(surface.TSR));
//	glDrawElements(GL_TRIANGLES, surface.index.size(), GL_UNSIGNED_INT, 0);
//}
// function to generate b-spline surface
float BasisFunction(int index, int degree, float t, vector<float> KnotVector) {
	if (degree == 0) {
		if (t >= KnotVector[index] && t < KnotVector[index + 1])return 1.0f;
		else return 0.0f;
	}

	float left{ 0.0f };
	float right{ 0.0f };

	if (KnotVector[index + degree] != KnotVector[index]) {
		left = (t - KnotVector[index]) / (KnotVector[index + degree] - KnotVector[index]) * BasisFunction(index, degree - 1, t, KnotVector);
	}
	if (KnotVector[index + degree + 1] != KnotVector[index + 1]) {
		right = (KnotVector[index + degree + 1] - t) / (KnotVector[index + degree + 1] - KnotVector[index + 1]) * BasisFunction(index + 1, degree - 1, t, KnotVector);
	}

	if (t == KnotVector.back() && index == KnotVector.size() - degree - 2)
		return 1.0f;

	return left + right;

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

inline bool intersectRayHexahedron(const Shape& Hexahedron, const glm::vec3& rayBegin, const glm::vec3& rayEnd,
	glm::vec3& intersectionPoint, float& distance, int& intersectedIndex) {
	bool intersected = false;
	float minDistance = FLT_MAX;

	for (int i = 0; i < Hexahedron.index.size(); i += 3) {
		glm::vec3 v0 = glm::vec3(Hexahedron.TSR * glm::vec4(Hexahedron.position[Hexahedron.index[i]], 1.0f));
		glm::vec3 v1 = glm::vec3(Hexahedron.TSR * glm::vec4(Hexahedron.position[Hexahedron.index[i + 1]], 1.0f));
		glm::vec3 v2 = glm::vec3(Hexahedron.TSR * glm::vec4(Hexahedron.position[Hexahedron.index[i + 2]], 1.0f));

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

vector<float> initKnotVector(int n, int degree) {
	int length = n + degree + 1;
	vector<float> KnotVector(length);

	int segment = n - degree;

	for (int i = 0; i < length; i++) {
		if (i <= degree) {
			KnotVector[i] = 0.0f;
		}
		else if (i >= n + 1) {
			KnotVector[i] = 1.0f;
		}
		else {
			KnotVector[i] = (float)(i - degree) / (float)segment;
		}
	}

	return KnotVector;
}

// normal vector of a, b
glm::vec3 getNormal(const glm::vec3& a, const glm::vec3& b) {
	return glm::normalize(glm::cross(a, b));
}

glm::vec3 RayfromMouse(mouseCoordGL mgl, const glm::mat4& ProjectionMatrix, const glm::mat4& view) {
	glm::mat4 inverseProjection = glm::inverse(ProjectionMatrix);
	glm::mat4 inverseView = glm::inverse(view);

	glm::vec4 clip_ray = glm::vec4(mgl.x, mgl.y, -1.0f, 1.0f);

	glm::vec4 view_ray = inverseProjection * clip_ray;
	view_ray = glm::vec4(view_ray.x, view_ray.y, -1.0f, 0.0f);

	glm::vec4 world_ray = inverseView * view_ray;

	return glm::normalize(glm::vec3(world_ray));
}

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

inline float perlinSmooth(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
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


void initSplineSurface(vector<vector<glm::vec3>>& ControlPoints, const int& nRows, const int& nCols) {

	SurfacePoints.clear();

	if (ControlPoints.size() != nRows) {
		std::cerr << "행 개수 다름" << std::endl;
		return;
	}
	for (const auto& row : ControlPoints) {
		if (row.size() != nCols) {
			std::cerr << "열 개수 다름" << std::endl;
			return;
		}
	}
	int u_degree = 3;
	int v_degree = 3;

	vector<float> KnotVectorU = initKnotVector(nRows, u_degree);
	vector<float> KnotVectorV = initKnotVector(nCols, v_degree);
	int sampleCount = (int)(1.0f / SAMPLE_INTERVAL) + 1;

	for (int p = 0; p < sampleCount; p++) {
		float u = p * SAMPLE_INTERVAL;
		for (int q = 0; q < sampleCount; q++) {
			float v = q * SAMPLE_INTERVAL;
			glm::vec3 CurrentPoint(0.0f, 0.0f, 0.0f);
			for (int i = 0; i < nRows; i++) {
				float PointU = BasisFunction(i, u_degree, u, KnotVectorU);
				for (int j = 0; j < nCols; j++) {
					float PointV = BasisFunction(j, v_degree, v, KnotVectorV);
					CurrentPoint += ControlPoints[i][j] * PointU * PointV;
				}
			}
			SurfacePoints.push_back(CurrentPoint);
		}
	}

	int stepU = (int)(1.0f / SAMPLE_INTERVAL) / (nRows - 1); 
	int stepV = (int)(1.0f / SAMPLE_INTERVAL) / (nCols - 1);

	ControlLines.clear();
	for (int i = 0; i < ControlPoints.size(); ++i) {
		for (int j = 0; j < ControlPoints[i].size(); ++j) {
			if (j + 1 < ControlPoints[i].size()) {
				Shape lineH(2);
				glm::vec3 white[2] = { glm::vec3(1.0f), glm::vec3(1.0f) };
				setLine(lineH, ControlPoints[i][j], ControlPoints[i][j + 1], white);
				ControlLines.push_back(lineH);
			}
			if (i + 1 < ControlPoints.size()) {
				Shape lineV(2);
				glm::vec3 white[2] = { glm::vec3(1.0f), glm::vec3(1.0f) };
				setLine(lineV, ControlPoints[i][j], ControlPoints[i + 1][j], white);
				ControlLines.push_back(lineV);
			}
		}
	}
	// 현재 랜더링되는 사각형들은 서로 정점을 공유함, 이 중복되는 정점
	

	int SizeU = sampleCount;
	int SizeV = sampleCount;

	vector<int> SurfaceIndices;
	for(int i = 0; i < SizeU - 1; i++) {
		for (int j = 0; j < SizeV - 1; j++) {
			// 사각형 정점 인덱스 중복 저장을 방지하기 위해 EBO 사용
			int index = i * SizeV + j;
			SurfaceIndices.push_back(index);
			SurfaceIndices.push_back(index + SizeV);
			SurfaceIndices.push_back(index + SizeV + 1);
			SurfaceIndices.push_back(index + SizeV + 1);
			SurfaceIndices.push_back(index + 1);
			SurfaceIndices.push_back(index);
		}
	}
	// 이전 방식에서는 같은 위치의 정점이라도 별개로 취급했으므로 각 면마다 노멀을 계산했음, 하지만 정점을 재사용하면 다른 노멀 계산이 필요함
	// face normal -> vertex normal
	/*1.모든 정점에 매핑 가능한 float3(vec3) 배열을 만든다
	2. 원래 가지고 있던 모든 정점의 배열과 각 삼각형을 정의하는 인덱스를 활용해서 노멀 계산
		(인덱스를 3씩 늘리면서 한 번 반복 index, index + 1, index + 2로 이루어진 노멀 계산)
	3. 그리고 그렇게 계산한 노멀을 각 계산의 반복(각 삼각형의 반복)마다 해당 3개 점에 대응하는 1.에서 만든 배열의 인덱스에 누산 +=
	4. 정점배열[n]에 대응하는 노멀배열[n]이 만들어진다, 정규화 필요*/

	vector<glm::vec3> SurfaceNormals(SurfacePoints.size(), glm::vec3(0.0f, 0.0f, 0.0f));
	for (int i = 0; i < SurfaceIndices.size(); i += 3) {
		glm::vec3 p0 = SurfacePoints[SurfaceIndices[i]];
		glm::vec3 p1 = SurfacePoints[SurfaceIndices[i + 1]];
		glm::vec3 p2 = SurfacePoints[SurfaceIndices[i + 2]];

		glm::vec3 normal = normalize(cross(glm::vec3(p1-p0), glm::vec3(p2-p0)));
		// 삼각형의 면적에 따른 normal에 가중치를 부여할 수 있음
		//glm::vec3 face = cross(p1 - p0, p2 - p0);
		//float area = length(face) * 0.5f;
		//glm::vec3 normal = normalize(face) * area;  == face(면)노멀 x 면적
		SurfaceNormals[SurfaceIndices[i]] += normal;
		SurfaceNormals[SurfaceIndices[i + 1]] += normal;
		SurfaceNormals[SurfaceIndices[i + 2]] += normal;
	}

	for(auto& normal : SurfaceNormals) {
		if (glm::dot(normal, normal) > 0.0f) normal = glm::normalize(normal);
	}

	glm::vec3 gray = glm::vec3(0.5f, 0.5f, 0.5f);
	setSurface(Surface, SurfacePoints, SurfaceIndices, SurfaceNormals, gray);

	/*rectangles.clear();
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
	cout << rectangles.size() << endl;*/

	float half = 0.02f;
	for (int i = 0; i < nRows; i++) {
		for (int j = 0; j < nCols; j++) {
			Shape cube(8);
			setHexahedron(cube, glm::vec3(0.0f, 0.0f, 0.0f), half, glm::vec3(1.0f, 0.0f, 1.0f));
			cube.linkedPosition = &ControlPoints[i][j];
			cube.linkedRows = i;
			cube.linkedCols = j;
			cube.TSR = glm::translate(glm::mat4(1.0f), ControlPoints[i][j]);
			v_ControlPoints.push_back(cube);
		}
	}

	for (int i = 0; i < SurfacePoints.size(); i++) {
		cout << "SurfacePoints[" << i << "]: " << SurfacePoints[i].x << ", " << SurfacePoints[i].y << ", " << SurfacePoints[i].z << endl;
	}
	for(int i = 0; i < SurfaceNormals.size(); i++) {
		cout << "SurfaceNormals[" << i << "]: " << SurfaceNormals[i].x << ", " << SurfaceNormals[i].y << ", " << SurfaceNormals[i].z << endl;
	}

}

GLvoid Keyboard(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera[0] += CameraForward * CameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera[0] -= CameraForward * CameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera[0] -= CameraRight * CameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera[0] += CameraRight * CameraSpeed;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void CallbackMouseButton(GLFWwindow* window, int button, int action, int mods) {
	bool GizmoActive = false;
	if(ImGuizmo::IsOver() && PickedControlPoint) {
		GizmoActive = true; // ImGuizmo가 활성화되어 있으면 마우스 이벤트를 무시
		//cout << "is over true" << endl;
	}
	if (ImGuizmo::IsUsing() && PickedControlPoint) {
		GizmoActive = true; // ImGuizmo가 사용 중이면 마우스 이벤트를 무시
		//cout << "is using true" << endl;
	}
	if (GizmoActive) return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		glfwGetCursorPos(window, &mgl.x, &mgl.y);
		glfwGetFramebufferSize(window, &FrameBufferWidth, &FrameBufferHeight);
		mgl = transformMouseToGL(mgl.x+0.5, mgl.y+0.5, FrameBufferWidth, FrameBufferHeight);
		glm::vec3 point = RayfromMouse(mgl, projection, view);
		glm::vec3 ray = camera[0] + point * 500.0f;

		/*Shape* temp = new Shape(2);
		glm::vec3 rayColor[2] = { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f) };

		setLine(*temp, camera[0], ray, rayColor);
		axes.push_back(*temp);
		delete(temp);*/

		glm::vec3 intersectionPoint;
		float distance = 0.0f;
		int intersectedIndex = -1;

		PickedControlPoint = NULL;
		PickedObjectModelTransform = glm::mat4(1.0f);
		for (auto& c : v_ControlPoints) {
			if (intersectRayHexahedron(c, camera[0], ray, intersectionPoint, distance, intersectedIndex)) {
				
				PickedObjectModelTransform = c.TSR;
				PickedControlPoint = &c;
				break;
			}
		}
		

	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		MouseRightButtonPressed = true;
		glfwGetCursorPos(window, &mgl.x, &mgl.y);
		mgl = transformMouseToGL(mgl.x, mgl.y, windowWidth, windowHeight);
		preMousePosition = mgl;
	}
	if( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		MouseRightButtonPressed = false;
		glfwGetCursorPos(window, &mgl.x, &mgl.y);
		mgl = transformMouseToGL(mgl.x, mgl.y, windowWidth, windowHeight);
		preMousePosition = mgl;
	}
}

void MouseMoveRightButton(GLFWwindow* window) {
	glfwGetCursorPos(window, &mgl.x, &mgl.y);
	mgl = transformMouseToGL(mgl.x, mgl.y, windowWidth, windowHeight);

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
}

void CallbackMouseMove(GLFWwindow* window, double xpos, double ypos) {

}

void CallbackMouseWheel(GLFWwindow* window, double xoffset, double yoffset) {
	if (MouseRightButtonPressed) {
		if (yoffset > 0 && CameraSpeed < 10.0f) CameraSpeed += 0.01f;
		else if (yoffset < 0 && CameraSpeed > 0.01f) CameraSpeed -= 0.01f;

	}
}	
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



void DrawPanel() {

	ImGuiIO& io = ImGui::GetIO();
	float panel_width = 300.0f;
	float button_height = 40.0f;

	// 패널 위치
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - panel_width, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(panel_width, io.DisplaySize.y), ImGuiCond_Always);

	ImGui::Begin("Right Panel", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove);

	// 스크롤 가능한 툴
	ImGui::BeginChild("ScrollableRegion", ImVec2(0, -button_height - 20), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

	ImGui::Text("modify constraint points");
	ImGui::Separator();
	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ modifiers
	if (ImGui::Button("Rows + 1", ImVec2(-FLT_MIN, 30))) {
		int addedRows = controlPoints_modifier.size() + 1;
		int addedCols = controlPoints_modifier[0].size();

		vector<vector<glm::vec3>> temp = MakeInitialControlPoints(addedRows, addedCols);
		for (int i = 0; i < controlPoints_modifier.size(); i++) {
			for (int j = 0; j < controlPoints_modifier[0].size(); j++) {
				temp[i][j] = controlPoints_modifier[i][j];
			}
		}
		controlPoints_modifier = temp;
	}
	if (ImGui::Button("Cols + 1", ImVec2(-FLT_MIN, 30))) {
		int addedRows = controlPoints_modifier.size();
		int addedCols = controlPoints_modifier[0].size() + 1;

		vector<vector<glm::vec3>> temp = MakeInitialControlPoints(addedRows, addedCols);
		for (int i = 0; i < controlPoints_modifier.size(); i++) {
			for (int j = 0; j < controlPoints_modifier[0].size(); j++) {
				temp[i][j] = controlPoints_modifier[i][j];
			}
		}
		controlPoints_modifier = temp;
	}

	// 선택된 제어점의 좌표 표시(left click으로 선택했을때만
	if (PickedControlPoint) {
		glm::vec3 pos = controlPoints_modifier[PickedControlPoint->linkedRows][PickedControlPoint->linkedCols];
		ImGui::Text("Selected: (%.3f, %.3f, %.3f)", pos.x, pos.y, pos.z);
	}

	int inputRows = controlPoints_modifier.size();
	int inputCols = controlPoints_modifier[0].size();
	if (ImGui::InputInt("Rows", &inputRows, 1, 100)){
		vector<vector<glm::vec3>> temp = MakeInitialControlPoints(inputRows, inputCols);
		if (inputRows < controlPoints_modifier.size()) {
			for (int i = 0; i < inputRows; i++) {
				for (int j = 0; j < controlPoints_modifier[0].size(); j++) {
					temp[i][j] = controlPoints_modifier[i][j];
				}
			}
		}
		else {
			for (int i = 0; i < controlPoints_modifier.size(); i++) {
				for (int j = 0; j < controlPoints_modifier[0].size(); j++){
					temp[i][j] = controlPoints_modifier[i][j];
				}
			}
		}
		controlPoints_modifier = temp;
	}
	if (ImGui::InputInt("Cols", &inputCols, 1, 100)) {
		vector<vector<glm::vec3>> temp = MakeInitialControlPoints(inputRows, inputCols);
		if (inputCols < controlPoints_modifier[0].size()) {
			for (int i = 0; i < controlPoints_modifier.size(); i++) {
				for (int j = 0; j < inputCols; j++) {
					temp[i][j] = controlPoints_modifier[i][j];
				}
			}
		}
		else {
			for (int i = 0; i < controlPoints_modifier.size(); i++) {
				for (int j = 0; j < controlPoints_modifier[0].size(); j++) {
					temp[i][j] = controlPoints_modifier[i][j];
				}
			}
		}
		controlPoints_modifier = temp;
	}


	ImGui::Text("modify noise");
	ImGui::Separator();
	ImGui::Button("Noise A", ImVec2(-FLT_MIN, 30));
	ImGui::Button("Noise B", ImVec2(-FLT_MIN, 30));

	ImGui::Text("Rendering");
	ImGui::Separator();
	if (ImGui::Button("enable controlpoint render", ImVec2(-FLT_MIN, 30))) {
		ControlPointRender = !ControlPointRender;
	}
	if(ImGui::Button("increase definition", ImVec2(-FLT_MIN, 30))){
		SAMPLE_INTERVAL -= 0.01f;
		if (SAMPLE_INTERVAL < 0.01f) SAMPLE_INTERVAL = 0.01f;
	}
	if(ImGui::Button("decrease definition", ImVec2(-FLT_MIN, 30))){
		SAMPLE_INTERVAL += 0.01f;
		if (SAMPLE_INTERVAL > 0.1f) SAMPLE_INTERVAL = 0.1f;
	}

	ImGui::EndChild();

	// 하단 confirm, reset 버튼
	ImGui::Separator();
	if (ImGui::Button("confirm", ImVec2((panel_width - 30) * 0.5f, button_height)))
	{
		rectangles.clear();
		v_ControlPoints.clear();
		initSplineSurface(controlPoints_modifier, controlPoints_modifier.size(), controlPoints_modifier[0].size());
	}
	ImGui::SameLine();
	if (ImGui::Button("reset", ImVec2((panel_width - 30) * 0.5f, button_height)))
	{
		cout << "reset surface" << endl;
		vector<vector<glm::vec3>> temp = MakeInitialControlPoints(controlPoints_modifier.size(), controlPoints_modifier[0].size());
		controlPoints_modifier = temp;
		initSplineSurface(temp, controlPoints_modifier.size(), controlPoints_modifier[0].size());
	}

	ImGui::End();
}

void UpdateToolInteraction() {
	/*switch (currentTool) {
	case ToolType::confirm:
		initSplineSurface(v_ControlPoints, nRows, nCols);
	}*/
}

vector<vector<glm::vec3>> MakeInitialControlPoints(const int& Rows, const int& Cols) {
	vector<vector<glm::vec3>> points(Rows, vector<glm::vec3>(Cols));

	for (int i = 0; i < Rows; i++) {
		for (int j = 0; j < Cols; j++) {
			points[i][j] = glm::vec3(j, 0.0f, i);
		}
	}

	return points;
}