#include "base.h"
#include "ShaderManager.h"
#include "BufferManager.h"
#include "Mesh.h"
#include "RenderManager.h"
#include "B_SplineSurface.h"
#include "HeightMap.h"
#include "NoiseGenerator.h"
#include "FeatureCurve.h"

ShaderManager ShaderMgr("vertex.glsl", "fragment.glsl");
BufferManager BufferMgr;
RenderManager RenderMgr(ShaderMgr, BufferMgr);

//float PI = 3.14159265358979323846f;

static random_device random;
static mt19937 gen(random());
static uniform_real_distribution<> distribution(0, 2.0 * PI);

bool MoveCameraForward = false;
bool MoveCameraBackward = false;
bool MoveCameraLeft = false;
bool MoveCameraRight = false;
float CameraYaw = -90.0f;
float CameraPitch = 0.0f;

char ControlMode = 0;
bool EditVectorMode = false;

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

bool DrawRightPanel = true;

bool DragMode = false;

int InputTangent = 0;

bool WireFrame = false;

int DrawCurveMode = 0;

float SAMPLE_INTERVAL = 1.0f/2.0f;
int sampleCount = 0.0f;

COLOR backgroundColor{ 1.0f, 1.0f, 1.0f, 0.0f };

mouseCoordGL mgl;
mouseCoordGL preMousePosition;

GLint width, height;

unsigned int viewLocation;
unsigned int projectionLocation;
unsigned int modelTransformLococation;

vector <Shape> axes;
vector <LineMesh> Axes;
// camera
glm::vec3 camera[3];
glm::vec3 CameraForward;
glm::vec3 CameraRight;
glm::mat4 view = glm::mat4(1.0f);


glm::mat4 projection = glm::mat4(1.0f);

vector<vector<glm::vec3>> controlPoints;
//vector<vector<glm::vec3>> controlPoints_modifier;
vector<vector<glm::vec3>> controlPoints_initial;
vector<Shape> ControlLines;
vector<glm::vec3> VerticesForControlLines;
vector<ConstraintPoint> constraintPoints;
vector<Shape> rectangles;						// render surface rectangles
vector<ControlPointVisualMesh> v_ControlPoints;				// render control points
ControlPointVisualMesh* PickedControlPoint{ NULL };	// picked control point

glm::vec3 GridPoints[TERRAIN_SIZE][TERRAIN_SIZE];
vector<Shape> GridLines;
vector<Shape> GridRectangles;

vector<glm::vec3> SurfacePoints;

glm::vec3 LightSource = glm::vec3(0.0f, 0.0f, 0.0f);

float DiffusionGrid[TERRAIN_SIZE - 1][TERRAIN_SIZE - 1];

float noiseMap[1024][1024];

ToolType CurrentTool = ToolType::none;

glm::mat4 PickedObjectModelTransform = glm::mat4(1.0f);

glm::vec3 PickedObjectPos = glm::vec3(0.0f, 0.0f, 0.0f);

TerrainMesh Surface(0);
TerrainMesh SurfaceWire(0);

LineMesh Lines(0);

HeightMap heightmap(0, 0);

B_SplineSurface SplineSurface(0, 0);

B_SplineSurface SplineSurface_Modifier(0, 0);

NoiseGenerator NoiseGen(0, 0);

FeatureCurveManager FeatureCurveMgr;


// basis function 캐싱
int CachedRows = 0;
int CachedCols = 0;
float CachedInterval = 0.0f;

vector<vector<float>> CachedBasisU;
vector<vector<float>> CachedBasisV;

static NoiseParameters noiseParameters;

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

	ShaderMgr.InitShader();

	RenderMgr.Init("viewTransform", "projectionTransform", "modelTransform", "lightPos");

	glUseProgram(ShaderMgr.GetShaderProgramID());

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

	//controlPoints_modifier = controlPoints;

	glfwSetScrollCallback(window, CallbackMouseWheel);
	if(DragMode)glfwSetCursorPosCallback(window, CallbackMouseMove);
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
			SplineSurface.SetControlPoint(PickedControlPoint->LinkedRow, PickedControlPoint->LinkedCol, newPosition);
			PickedObjectPos = newPosition;
		}

		DrawPanel();
		DrawMouseOverlay(window);

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

	viewLocation = glGetUniformLocation(ShaderMgr.GetShaderProgramID(), "viewTransform");
	projectionLocation = glGetUniformLocation(ShaderMgr.GetShaderProgramID(), "projectionTransform");
	modelTransformLococation = glGetUniformLocation(ShaderMgr.GetShaderProgramID(), "modelTransform");

	unsigned int lightSourceLocation = glGetUniformLocation(ShaderMgr.GetShaderProgramID(), "lightPos");

	// axes
	/*Shape* temp = new Shape(2);
	setLine(ShaderMgr.GetShaderProgramID(), *temp, glm::vec3(-4.0, 0.0, 0.0), glm::vec3(4.0, 0.0, 0.0), returnColorRand2());
	axes.push_back(*temp);
	setLine(ShaderMgr.GetShaderProgramID(), *temp, glm::vec3(0.0, -4.0, 0.0), glm::vec3(0.0, 4.0, 0.0), returnColorRand2());
	axes.push_back(*temp);
	setLine(ShaderMgr.GetShaderProgramID(), *temp, glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 0.0, 4.0), returnColorRand2());
	axes.push_back(*temp);
	delete(temp);*/

	// axes - LineMesh version

	LineMesh *temp2 = new LineMesh(2);
	temp2->SetLine(glm::vec3(-4.0, 0.0, 0.0), glm::vec3(4.0, 0.0, 0.0), returnColorRand2());
	temp2->SetMeshID(BufferMgr.CreateMeshID());
	BufferMgr.CreateBufferData(temp2->GetMeshID(), false);
	// 여기서 정점 업로드, BufferMgr.BindVertexBufferObjectByID() 호출
	BufferMgr.BindVertexBufferObjectByID(temp2->GetMeshID(), temp2->GetPosition().data(), temp2->GetPosition().size(),
		temp2->GetColor().data(), temp2->GetColor().size(),
		nullptr, 0);
	
	Axes.push_back(*temp2);

	temp2->SetLine(glm::vec3(0.0, -4.0, 0.0), glm::vec3(0.0, 4.0, 0.0), returnColorRand2());
	temp2->SetMeshID(BufferMgr.CreateMeshID());
	BufferMgr.CreateBufferData(temp2->GetMeshID(), false);
	// 여기서 정점 업로드, BufferMgr.BindVertexBufferObjectByID() 호출

	BufferMgr.BindVertexBufferObjectByID(temp2->GetMeshID(), temp2->GetPosition().data(), temp2->GetPosition().size(),
		temp2->GetColor().data(), temp2->GetColor().size(),
		nullptr, 0);

	Axes.push_back(*temp2);

	temp2->SetLine(glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 0.0, 4.0), returnColorRand2());
	temp2->SetMeshID(BufferMgr.CreateMeshID());
	BufferMgr.CreateBufferData(temp2->GetMeshID(), false);
	// 여기서 정점 업로드, BufferMgr.BindVertexBufferObjectByID() 호출
	BufferMgr.BindVertexBufferObjectByID(temp2->GetMeshID(), temp2->GetPosition().data(), temp2->GetPosition().size(),
		temp2->GetColor().data(), temp2->GetColor().size(),
		nullptr, 0);
	Axes.push_back(*temp2);

	//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

	// ㅡㅡ init heightmap and spline surface ㅡㅡ

	heightmap.SetResolution(1024, 1024);
	SplineSurface.SetResolution(heightmap.GetResU(), heightmap.GetResV());
	SplineSurface.GenerateSurface();
	heightmap.SetHeight(SplineSurface.GetHeightMap());

	NoiseGen.SetRes(1024, 1024);

	controlPoints_initial = {
		{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1 / 3.0f, 0.0f, 0.0f), glm::vec3(2 / 3.0f, 0.0f, 0.0f) , glm::vec3(3 / 3.0f, 0.0f, 0.0f)},
		{glm::vec3(0.0f, 0.0f, 1 / 3.0f), glm::vec3(1 / 3.0f, 0.0f, 1 / 3.0f), glm::vec3(2 / 3.0f, 0.0f, 1 / 3.0f) , glm::vec3(3 / 3.0f, 0.0f, 1 / 3.0f)},
		{glm::vec3(0.0f, 0.0f, 2 / 3.0f), glm::vec3(1 / 3.0f, 0.0f, 2 / 3.0f), glm::vec3(2 / 3.0f, 0.0f, 2 / 3.0f) , glm::vec3(3 / 3.0f, 0.0f, 2 / 3.0f)},
		{glm::vec3(0.0f, 0.0f, 3 / 3.0f), glm::vec3(1 / 3.0f, 0.0f, 3 / 3.0f), glm::vec3(2 / 3.0f, 0.0f, 3 / 3.0f) , glm::vec3(3 / 3.0f, 0.0f, 3 / 3.0f)},
	};

	initSplineSurface();

	// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

	delete(temp2);

	camera[0] = glm::vec3(0.4, 0.2, 0.8);
	camera[1] = glm::vec3(0.5, 0.0, 0.5);
	camera[2] = glm::vec3(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(camera[0], camera[1], camera[2]);
	CameraForward = camera[1] - camera[0];
	CameraRight = glm::normalize(glm::cross(CameraForward, camera[2]));


	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 500.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glLineWidth(1.5f);

}

GLvoid drawScene() {

	//std::cout << Surface.GetMeshID() << "SURFACEMESHID" << endl;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glDisable(GL_DEPTH_TEST);
	/*glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);*/


	float aspect = static_cast<float>(windowWidth) / windowHeight;
	projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 500.0f);

	RenderMgr.BeginFrame(view, projection, LightSource);

	for (const auto& a : Axes) RenderMgr.Draw(a);

	for (const auto& fc : FeatureCurveMgr.GetCurves()) {
		for (const auto& cp : fc.GetControlPoints()) {
			if (cp.GetMesh()) {
				RenderMgr.Draw(*cp.GetMesh());
			}
		}
	}
	
	//draw(rectangles);

	//RenderMgr.Draw(Surface);

	//draw(ShaderMgr.GetShaderProgramID(), Surface);

	//if (WireFrame) drawWireframe(ShaderMgr.GetShaderProgramID(), SurfaceWire);

	if (WireFrame) RenderMgr.DrawWireframe(Surface);
	else RenderMgr.Draw(Surface);


	if (ControlPointRender) {
		
		for (const auto& cp : v_ControlPoints) {
			RenderMgr.Draw(cp);
		}
		RenderMgr.Draw(Lines);
		glEnable(GL_DEPTH_TEST);

	}




	view = glm::lookAt(camera[0], camera[0] + CameraForward, camera[2]);

	

	/*glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);*/

	UpdateWindowTitleWithFPS(glfwGetCurrentContext());
}


// normal vector of a, b




void initSplineSurface() {

	auto start = chrono::high_resolution_clock::now();

	SurfacePoints.clear();

	LineMesh Line(VerticesForControlLines.size());
	Line.SetMeshID(BufferMgr.CreateMeshID());
	BufferMgr.CreateBufferData(Line.GetMeshID(), false);

	glm::vec3 w = glm::vec3(1.0f);
	Line.SetLines(VerticesForControlLines, w);
	BufferMgr.BindVertexBufferObjectByID(Line.GetMeshID(), Line.GetPosition().data(), Line.GetPosition().size(), Line.GetColor().data(), Line.GetColor().size(),
		nullptr, 0);

	Lines = Line;





	VerticesForControlLines.clear();
	//VerticesForControlLines.resize(nRows * nCols);


	glm::vec3 gray = glm::vec3(0.5f, 0.5f, 0.5f);
	Surface.SetSurfaceNormalized(heightmap.GetHeightMap(), heightmap.GetResU(), heightmap.GetResV(), gray);
	//Surface.SetSurface(SurfacePoints, SurfaceIndices, SurfaceNormals, gray);
	Surface.SetMeshID(BufferMgr.CreateMeshID());
	BufferMgr.CreateBufferData(Surface.GetMeshID(), true);
	BufferMgr.BindVertexBufferObjectByID(Surface.GetMeshID(), Surface.GetPosition().data(), Surface.GetPosition().size(),
		Surface.GetColor().data(), Surface.GetColor().size(), Surface.GetNormal().data(), Surface.GetNormal().size());
	BufferMgr.BindElementBufferObjectByID(Surface.GetMeshID(), Surface.GetIndex().data(), Surface.GetIndex().size());



	glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
	Surface.SetSurfaceNormalized(heightmap.GetHeightMap(), heightmap.GetResU(), heightmap.GetResV(), white);
	//SurfaceWire.SetSurface(SurfacePoints, SurfaceIndices, SurfaceNormals, white);
	SurfaceWire.SetMeshID(BufferMgr.CreateMeshID());
	BufferMgr.CreateBufferData(SurfaceWire.GetMeshID(), true);
	BufferMgr.BindVertexBufferObjectByID(SurfaceWire.GetMeshID(), SurfaceWire.GetPosition().data(), SurfaceWire.GetPosition().size(),
		SurfaceWire.GetColor().data(), SurfaceWire.GetColor().size(), SurfaceWire.GetNormal().data(), SurfaceWire.GetNormal().size());
	BufferMgr.BindElementBufferObjectByID(SurfaceWire.GetMeshID(), SurfaceWire.GetIndex().data(), SurfaceWire.GetIndex().size());


	float half = 0.03f;
	for (int i = 0; i < SplineSurface.GetRowsControlPoints(); i++) {
		for (int j = 0; j < SplineSurface.GetColsControlPoints(); j++) {
			ControlPointVisualMesh Cube(8);
			Cube.SetHexahedron(glm::vec3(0.0f, 0.0f, 0.0f), half, glm::vec3(1.0f, 0.0f, 1.0f));
			Cube.LinkedPosition = SplineSurface.GetControlPoint(i, j);
			Cube.LinkedRow = i;
			Cube.LinkedCol = j;
			Cube.TSR = glm::translate(glm::mat4(1.0f), SplineSurface.GetControlPoint(i, j));

			Cube.SetMeshID(BufferMgr.CreateMeshID());
			BufferMgr.CreateBufferData(Cube.GetMeshID(), true);
			BufferMgr.BindVertexBufferObjectByID(Cube.GetMeshID(), Cube.GetPosition().data(), Cube.GetPosition().size(),
				Cube.GetColor().data(), Cube.GetColor().size(), nullptr, 0);
			BufferMgr.BindElementBufferObjectByID(Cube.GetMeshID(), Cube.GetIndex().data(), Cube.GetIndex().size());

			v_ControlPoints.push_back(Cube);
		}
	}

	auto end = chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;

	std::cout << "생성 시간: " << duration.count() << " ms\n";

	//for (int i = 0; i < SurfacePoints.size(); i++) {
	//	cout << "SurfacePoints[" << i << "]: " << SurfacePoints[i].x << ", " << SurfacePoints[i].y << ", " << SurfacePoints[i].z << endl;
	//}
	//for(int i = 0; i < SurfaceNormals.size(); i++) {
	//	cout << "SurfaceNormals[" << i << "]: " << SurfaceNormals[i].x << ", " << SurfaceNormals[i].y << ", " << SurfaceNormals[i].z << endl;
	//}

	/*cout << sampleCount * sampleCount << endl;
	cout << SAMPLE_INTERVAL << endl;*/



}

void UpdateSplineSurface() {

	LineMesh Line(VerticesForControlLines.size());
	Line.SetMeshID(BufferMgr.CreateMeshID());
	BufferMgr.CreateBufferData(Line.GetMeshID(), false);

	glm::vec3 w = glm::vec3(1.0f);
	Line.SetLines(VerticesForControlLines, w);
	BufferMgr.BindVertexBufferObjectByID(Line.GetMeshID(), Line.GetPosition().data(), Line.GetPosition().size(), Line.GetColor().data(), Line.GetColor().size(),
		nullptr, 0);

	Lines = Line;

	VerticesForControlLines.clear();
	//VerticesForControlLines.resize(nRows * nCols);


	glm::vec3 gray = glm::vec3(0.5f, 0.5f, 0.5f);
	Surface.SetSurfaceNormalized(heightmap.GetHeightMap(), heightmap.GetResU(), heightmap.GetResV(), gray);
	//Surface.SetSurface(SurfacePoints, SurfaceIndices, SurfaceNormals, gray);
	Surface.SetMeshID(BufferMgr.CreateMeshID());
	BufferMgr.CreateBufferData(Surface.GetMeshID(), true);
	BufferMgr.BindVertexBufferObjectByID(Surface.GetMeshID(), Surface.GetPosition().data(), Surface.GetPosition().size(),
		Surface.GetColor().data(), Surface.GetColor().size(), Surface.GetNormal().data(), Surface.GetNormal().size());
	BufferMgr.BindElementBufferObjectByID(Surface.GetMeshID(), Surface.GetIndex().data(), Surface.GetIndex().size());



	glm::vec3 white = glm::vec3(1.0f, 1.0f, 1.0f);
	Surface.SetSurfaceNormalized(heightmap.GetHeightMap(), heightmap.GetResU(), heightmap.GetResV(), white);
	//SurfaceWire.SetSurface(SurfacePoints, SurfaceIndices, SurfaceNormals, white);
	SurfaceWire.SetMeshID(BufferMgr.CreateMeshID());
	BufferMgr.CreateBufferData(SurfaceWire.GetMeshID(), true);
	BufferMgr.BindVertexBufferObjectByID(SurfaceWire.GetMeshID(), SurfaceWire.GetPosition().data(), SurfaceWire.GetPosition().size(),
		SurfaceWire.GetColor().data(), SurfaceWire.GetColor().size(), SurfaceWire.GetNormal().data(), SurfaceWire.GetNormal().size());
	BufferMgr.BindElementBufferObjectByID(SurfaceWire.GetMeshID(), SurfaceWire.GetIndex().data(), SurfaceWire.GetIndex().size());

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

		if (!EditVectorMode) {
			glm::vec3 intersectionPoint;
			float distance = 0.0f;
			int intersectedIndex = -1;

			PickedControlPoint = NULL;
			PickedObjectModelTransform = glm::mat4(1.0f);
			for (auto& c : v_ControlPoints) {
				/*if (intersectRayHexahedron(c, camera[0], ray, intersectionPoint, distance, intersectedIndex)) {

					PickedObjectModelTransform = c.TSR;
					PickedControlPoint = &c;
					break;
				}*/
				if (intersectRayHexahedron(c, camera[0], ray, intersectionPoint, distance, intersectedIndex)) {
					PickedObjectModelTransform = c.TSR;
					PickedControlPoint = &c;
					break;
				}

			}
			if (!PickedControlPoint) {
				DragMode = true;
				preMousePosition = mgl;
			}
		}

		else if (EditVectorMode) {

			float t;

			glm::vec3 dir = ray - camera[0];
			t = -camera[0].y / dir.y;
			glm::vec3 hit = camera[0] + dir * t;

			glm::vec3 NodePos = glm::vec3(hit.x, 0.0f, hit.z);

			FeatureCurveMgr.LeftClick(NodePos, InputTangent);
			FeatureCurveMgr.UploadBuffers(BufferMgr);

			InputTangent = 0;

		}
		

	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		DragMode = false;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if (EditVectorMode)return;
		MouseRightButtonPressed = true;
		glfwGetCursorPos(window, &mgl.x, &mgl.y);
		mgl = transformMouseToGL(mgl.x, mgl.y, windowWidth, windowHeight);
		preMousePosition = mgl;
	}
	if( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		if (EditVectorMode)return;
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
	if (!DragMode) return;
	double startX = preMousePosition.x;
	double startY = preMousePosition.y;
	double endX = xpos;
	double endY = ypos;



}

void CallbackMouseWheel(GLFWwindow* window, double xoffset, double yoffset) {
	if (MouseRightButtonPressed) {
		if (yoffset > 0 && CameraSpeed < 10.0f) CameraSpeed += 0.01f;
		else if (yoffset < 0 && CameraSpeed > 0.01f) CameraSpeed -= 0.01f;
		return;
	}
	
	else if (EditVectorMode) {
		if (yoffset > 0 && InputTangent < 81) InputTangent += 1;
		else if (yoffset < 0 && InputTangent > -80) InputTangent -= 1;
	}
}	

void CameraTopCenter() {
	camera[0] = glm::vec3(0.5f, 1.5f, 0.5f);
	camera[1] = glm::vec3(0.5f, 0.0f, 0.5f);
	camera[2] = glm::vec3(0.0f, 0.0f, -1.0f);
	CameraForward = glm::vec3(0.0f, -1.0f, 0.0f);
	CameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
	CameraYaw = -90.0f;
	CameraPitch = -89.0f;
	CameraSpeed = 0.1f;
}

void CameraPerspective() {
	camera[0] = glm::vec3(0.4, 0.2, 0.8);
	camera[1] = glm::vec3(0.5, 0.0, 0.5);
	camera[2] = glm::vec3(0.0f, 1.0f, 0.0f);
	CameraYaw = -45.0f;
	CameraPitch = -20.0f;
	CameraSpeed = 0.1f;
}


void DrawPanel() {

	ImGuiIO& io = ImGui::GetIO();

	if (DrawRightPanel) {

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
		//ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ modifiers

	   // 선택된 제어점의 좌표 표시(left click으로 선택했을때만
		if (PickedControlPoint) {
			glm::vec3 pos = SplineSurface.GetControlPoint(PickedControlPoint->LinkedRow, PickedControlPoint->LinkedCol);
			ImGui::Text("Selected: (%.3f, %.3f, %.3f)", pos.x, pos.y, pos.z);
		}


		ImGui::Text("modify noise");
		ImGui::Separator();
		if (ImGui::Button("Random Value Noise", ImVec2(-FLT_MIN, 30))) {

		}

		if (ImGui::InputFloat("frequency", &noiseParameters.frequency, 0.1f, 1.0f));
		if (ImGui::InputInt("octaves", &noiseParameters.octaves, 1, 5));
		if (ImGui::InputFloat("persistence", &noiseParameters.persistence, 0.1f, 1.0f));
		if (ImGui::InputFloat("lacunarity", &noiseParameters.lacunarity, 0.1f, 1.0f));

		if (ImGui::Button("Perlin Noise", ImVec2(-FLT_MIN, 30))) {

			NoiseGen.GeneratePerlinNoise(
				16542,
				noiseParameters.frequency,
				noiseParameters.octaves,
				noiseParameters.persistence,
				noiseParameters.lacunarity);

		}
		if (ImGui::Button("Simplex Noise", ImVec2(-FLT_MIN, 30))) {

			NoiseGen.GenerateSimplexNoise(
				16542,
				noiseParameters.frequency,
				noiseParameters.octaves,
				noiseParameters.persistence,
				noiseParameters.lacunarity);

		}

		ImGui::Text("Rendering");
		ImGui::Separator();
		if (ImGui::Button("enable controlpoint render", ImVec2(-FLT_MIN, 30))) {
			ControlPointRender = !ControlPointRender;
		}
		if (ImGui::Button("enable surface wire render", ImVec2(-FLT_MIN, 30))) {
			WireFrame = !WireFrame;
		}
		if (ImGui::Button("increase resolution", ImVec2(-FLT_MIN, 30))) {
			SAMPLE_INTERVAL *= 1.0f / 2.0f;
			if (SAMPLE_INTERVAL < 1.0f / 1024.0f) SAMPLE_INTERVAL = 1.0f / 1024.0f;
		}
		if (ImGui::Button("decrease resolution", ImVec2(-FLT_MIN, 30))) {
			SAMPLE_INTERVAL *= 2.0f;
			if (SAMPLE_INTERVAL > 1.0f / 2.0f) SAMPLE_INTERVAL = 1.0f / 2.0f;
		}
		if (ImGui::Button("lighting", ImVec2(-FLT_MIN, 30))) {
			if (LightSource == glm::vec3(0.0f, 0.0f, 0.0f))LightSource = glm::vec3(0.0f, 10.0f, 0.0f);
			else LightSource = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		ImGui::EndChild();

		// 하단 confirm, reset 버튼
		ImGui::Separator();
		if (ImGui::Button("confirm", ImVec2((panel_width - 30) * 0.5f, button_height)))
		{
			SplineSurface.GenerateSurface();
			heightmap.AddHeight(SplineSurface.GetHeightMap());
			heightmap.AddHeight(NoiseGen.GetHeightMap());
			UpdateSplineSurface();
		}
		ImGui::SameLine();
		if (ImGui::Button("reset", ImVec2((panel_width - 30) * 0.5f, button_height)))
		{
			cout << "reset surface" << endl;
			SplineSurface.ResetControlPoints();
			heightmap.ClearHeight();
			initSplineSurface();
		}

		ImGui::Separator();
		if (ImGui::Button("Export", ImVec2((panel_width), button_height))) {
			ExportHeightMap("heightmap..r16");
			cout << "Exported heightmap.r16" << endl;
		}

		ImGui::End();
	}


	// 다른 패널
	io = ImGui::GetIO();
	ImVec2 pos = ImVec2(20.0f, 20.0f);

	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.2f);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::Begin("##FloatingButton", nullptr, flags);

	if (ImGui::Button("Edit Feature Curves")) {
		if (!EditVectorMode) {
			EditVectorMode = true;
			ControlPointRender = false;
			CameraTopCenter();
			CameraSpeed = 0.0f;
			
		}
		else {
			EditVectorMode = false;
			CameraPerspective();
			CameraSpeed = 0.01f;
		}
		
		
	}
	ImGui::SameLine();

	if(ImGui::Button("Draw Right Panel")) {
		DrawRightPanel = !DrawRightPanel;
	}
	ImGui::End();
}

void DrawMouseOverlay(GLFWwindow* window) {
	ImGuiIO& io = ImGui::GetIO();

	if (!EditVectorMode) return;

	ImVec2 p = io.MousePos + ImVec2(16.0f, 16.0f);

	ImGui::SetNextWindowPos(p, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.75f);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav |
	    ImGuiWindowFlags_NoInputs;

	glfwGetCursorPos(window, &mgl.x, &mgl.y);	
	glfwGetFramebufferSize(window, &FrameBufferWidth, &FrameBufferHeight);
	mgl = transformMouseToGL(mgl.x + 0.5, mgl.y + 0.5, FrameBufferWidth, FrameBufferHeight);
	glm::vec3 point = RayfromMouse(mgl, projection, view);
	glm::vec3 ray = camera[0] + point * 500.0f;
	glm::vec3 dir = ray - camera[0];
	float t = -camera[0].y / dir.y;
	glm::vec3 hit = camera[0] + dir * t;

	ImGui::Begin("##MouseOverlay", nullptr, flags);

	ImGui::Text("(%f, %f)", hit.x, hit.z);

	ImGui::Text("Tangent: %d", InputTangent);

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

void ExportHeightMap(const char* FileName) {

	ofstream file(FileName, ios::binary);

	for (int x = 0; x < sampleCount; x++) {
		for (int z = 0; z < sampleCount; z++) {
			float height = SurfacePoints[x * sampleCount + z].y;
			height = glm::clamp(height, 0.0f, 1.0f);
			uint16_t height16 = static_cast<uint16_t>(height * 65535.0f);
			file.write(reinterpret_cast<const char*>(&height16), sizeof(height16));
		}
	}

	file.close();
}

