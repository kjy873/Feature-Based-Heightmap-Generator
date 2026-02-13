#include "base.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

ShaderManager ShaderMgr("vertex.glsl", "fragment.glsl");
BufferManager BufferMgr;
RenderManager RenderMgr(ShaderMgr, BufferMgr);
DiffuseManager DiffuseMgr;

//float PI = 3.14159265358979323846f;

static random_device random;
static mt19937 gen(random());
static uniform_real_distribution<> distribution(0, 2.0 * PI);

DebugMode CurrentDebugMode = DebugMode::None;
float DebugOverlayAlpha = 0.0f;

int HeightMapU = 256;
int HeightMapV = 256;

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

bool ControlPointRender = false;

double windowWidth = 1920;
double windowHeight = 1080;
int FrameBufferWidth;
int FrameBufferHeight;
const float defaultSize = 0.05;

int ElevationIteration = 5;
int GradientIteration = 5000;

float CameraSpeed = 0.01f;

bool DrawRightPanel = true;

bool DragMode = false;

bool ControlPressed = false;
bool ShiftPressed = false;

int InputTangent = 0;

bool WireFrame = false;

int DrawCurveMode = 0;

bool UiWantMouse = false;
bool UiWantKeyboard = false;

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

Rasterizer RasterizerMgr;


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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
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

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // 에러 발생 즉시 콜백 호출 (디버깅 용이)
	glDebugMessageCallback(MessageCallback, 0);

    // 뷰포트 설정
	int fbw, fbh;
	glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

	ShaderMgr.InitShader();

	RenderMgr.Init("viewTransform", "projectionTransform", "modelTransform", "lightPos", "HighlightWeight", "DebugMode", "OverlayAlpha", "Res");

	glUseProgram(ShaderMgr.GetShaderProgramID());

	glfwGetFramebufferSize(window, &FrameBufferWidth, &FrameBufferHeight);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontFromFileTTF("fonts/NotoSansKR-Regular.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesKorean());

	

	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	init();

	//controlPoints_modifier = controlPoints;

	glfwSetScrollCallback(window, CallbackMouseWheel);
	if(DragMode)glfwSetCursorPosCallback(window, CallbackMouseMove);
    // 루프
    while (!glfwWindowShouldClose(window)) {

		UiWantMouse = io.WantCaptureMouse;
		UiWantKeyboard = io.WantCaptureKeyboard;


        // 입력 처리 (예: ESC 키)
		Keyboard(window);
		if (ControlPressed && EditVectorMode) HoveringWithCtrlInEditVector(window);
		if (ShiftPressed && EditVectorMode) HoveringWithShiftInEditVector(window);
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


		if (EditVectorMode) CurveManagerViewer();


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

	heightmap.SetResolution(HeightMapU, HeightMapV);
	SplineSurface.SetResolution(heightmap.GetResU(), heightmap.GetResV());
	SplineSurface.GenerateSurface();
	heightmap.SetHeight(SplineSurface.GetHeightMap());

	NoiseGen.SetRes(HeightMapU, HeightMapV);


	ShaderMgr.InitComputePrograms("Gradient.comp", "Elevation.comp", "Noise.comp", "Multigrid.comp");

	DiffuseMgr.Initialize(HeightMapU, HeightMapV);

	BufferMgr.CreateDebugTextures(HeightMapU, HeightMapV);

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

	RenderMgr.BeginFrame(view, projection, LightSource, CurrentDebugMode, DebugOverlayAlpha, HeightMapU, HeightMapV);

	BufferMgr.BindDebugTextures(ShaderMgr.GetShaderProgramID());

	for (const auto& a : Axes) RenderMgr.Draw(a);

	if (EditVectorMode) {
		if (ControlPressed) {
			if (!FeatureCurveMgr.GetHoveringControlPoint().GetMesh()) std::cout << "NULL Hovering Control Point Mesh" << std::endl;
			else {
				RenderMgr.UploadHighlightWeight(FeatureCurveMgr.GetHoveringControlPoint().GetHighlightWeight());
				RenderMgr.Draw(*FeatureCurveMgr.GetHoveringControlPoint().GetMesh());
			}
		}
		else if (ShiftPressed) {
			if (!FeatureCurveMgr.GetHoveringConstraintPoint().GetMesh()) std::cout << "NULL Hovering Constraint Point Mesh" << std::endl;
			else {
				RenderMgr.UploadHighlightWeight(FeatureCurveMgr.GetHoveringConstraintPoint().GetHighlightWeight());
				RenderMgr.Draw(*FeatureCurveMgr.GetHoveringConstraintPoint().GetMesh());
			}
		}
	}

	for (auto& fc : FeatureCurveMgr.GetCurves()) {

		for (const auto& cp : fc.GetControlPoints()) {
			if (cp.GetMesh()) {
				RenderMgr.UploadHighlightWeight(cp.GetHighlightWeight());
				RenderMgr.Draw(*cp.GetMesh());
			}
		}
		for (const auto& csp : fc.GetConstraintPoints()) {
			if (csp.GetMesh()) {
				RenderMgr.UploadHighlightWeight(csp.GetHighlightWeight());
				RenderMgr.Draw(*csp.GetMesh());
			}
		}

		RenderMgr.UploadHighlightWeight(fc.GetHighlightWeight());
		const LineMesh* Line = fc.GetLineMesh();
		if (!Line) continue;
		if (Line->GetPosition().size() == 0) continue;
		if (!Line->GetMeshID()) continue;
		RenderMgr.Draw(*Line);


	}

	
	//if (FeatureCurveMgr.GetPendedControlPoint().GetMesh()) RenderMgr.Draw(*FeatureCurveMgr.GetPendedControlPoint().GetMesh());
	if (FeatureCurveMgr.GetPendedControlPoint().has_value()) RenderMgr.Draw(*FeatureCurveMgr.GetPendedControlPoint()->GetMesh());
	//draw(rectangles);

	//RenderMgr.Draw(Surface);

	//draw(ShaderMgr.GetShaderProgramID(), Surface);

	//if (WireFrame) drawWireframe(ShaderMgr.GetShaderProgramID(), SurfaceWire);

	RenderMgr.UploadHighlightWeight(0.0f);

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

	//for (int i = 0; i < SplineSurface.GetRowsControlPoints(); i++) {
	//	for (int j = 0; j < SplineSurface.GetColsControlPoints(); j++) {
	//		VerticesForControlLines.push_back(SplineSurface.GetControlPoint(i, j));
	//	}
	//}

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

	//for (int i = 0; i < SplineSurface.GetRowsControlPoints(); i++) {
	//	for (int j = 0; j < SplineSurface.GetColsControlPoints(); j++) {
	//		VerticesForControlLines.push_back(SplineSurface.GetControlPoint(i, j));
	//	}
	//}

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

	if (UiWantKeyboard)return;

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

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ControlPressed = true;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) ControlPressed = false;

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ShiftPressed = true;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) ShiftPressed = false;
}

void CallbackMouseButton(GLFWwindow* window, int button, int action, int mods) {

	if (UiWantMouse)return;

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

		int ww, wh;
		glfwGetWindowSize(window, &ww, &wh);

		glfwGetFramebufferSize(window, &FrameBufferWidth, &FrameBufferHeight);

		double sx = mgl.x * (static_cast<double>(FrameBufferWidth) / static_cast<double>(ww));
		double sy = mgl.y * (static_cast<double>(FrameBufferHeight) / static_cast<double>(wh));

		mgl = transformMouseToGL(sx, sy, FrameBufferWidth, FrameBufferHeight);
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

			if (ControlPressed) {
				FeatureCurveMgr.Click(NodePos, InputButton::Left, InputMode::Ctrl);

				FeatureCurveMgr.UploadBuffers(BufferMgr);
				FeatureCurveMgr.UploadPendedBuffer(BufferMgr);

			}
			
			else if (ShiftPressed) {
				FeatureCurveMgr.Click(NodePos, InputButton::Left, InputMode::Shift);
				FeatureCurveMgr.UploadBuffers(BufferMgr);
				FeatureCurveMgr.UploadPendedBuffer(BufferMgr);
			}
			
			else {
				FeatureCurveMgr.Click(NodePos, InputButton::Left, InputMode::Default);
			}

		}

		

	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		DragMode = false;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {

		if (!EditVectorMode) {
			MouseRightButtonPressed = true;
			glfwGetCursorPos(window, &mgl.x, &mgl.y);
			mgl = transformMouseToGL(mgl.x, mgl.y, windowWidth, windowHeight);
			preMousePosition = mgl;
		}

		else if (EditVectorMode) {
			FeatureCurveMgr.Click(glm::vec3(0.0f, 0.0f, 0.0f), InputButton::Right, InputMode::Default);
		}
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

void HoveringWithCtrlInEditVector(GLFWwindow* window){

	if (FeatureCurveMgr.GetHoveringControlPointDirty()) {
		ControlPointVisualMesh* HoveringMesh = FeatureCurveMgr.GetHoveringControlPoint().GetMesh();
		HoveringMesh->SetMeshID(BufferMgr.CreateMeshID());
		BufferMgr.CreateBufferData(HoveringMesh->GetMeshID(), true);
		BufferMgr.BindVertexBufferObjectByID(HoveringMesh->GetMeshID(), HoveringMesh->GetPosition().data(), HoveringMesh->GetPosition().size(),
			HoveringMesh->GetColor().data(), HoveringMesh->GetColor().size(), nullptr, 0);
		BufferMgr.BindElementBufferObjectByID(HoveringMesh->GetMeshID(), HoveringMesh->GetIndex().data(), HoveringMesh->GetIndex().size());
		FeatureCurveMgr.SetHoveringControlPointDirty(false);

	}

	glfwGetCursorPos(window, &mgl.x, &mgl.y);

	int ww, wh;
	glfwGetWindowSize(window, &ww, &wh);

	glfwGetFramebufferSize(window, &FrameBufferWidth, &FrameBufferHeight);

	double sx = mgl.x * (static_cast<double>(FrameBufferWidth) / static_cast<double>(ww));
	double sy = mgl.y * (static_cast<double>(FrameBufferHeight) / static_cast<double>(wh));

	mgl = transformMouseToGL(sx, sy, FrameBufferWidth, FrameBufferHeight);
	glm::vec3 point = RayfromMouse(mgl, projection, view);
	glm::vec3 ray = camera[0] + point * 500.0f;

	float t;

	glm::vec3 dir = ray - camera[0];
	t = -camera[0].y / dir.y;
	glm::vec3 hit = camera[0] + dir * t;

	glm::vec3 Pos = glm::vec3(hit.x, 0.0f, hit.z);

	FeatureCurveMgr.HoverPressedCtrl(Pos);
}

void HoveringWithShiftInEditVector(GLFWwindow* window) {

	if (FeatureCurveMgr.GetHoveringConstraintPointDirty()) {
		ControlPointVisualMesh* HoveringMesh = FeatureCurveMgr.GetHoveringConstraintPoint().GetMesh();
		HoveringMesh->SetMeshID(BufferMgr.CreateMeshID());
		BufferMgr.CreateBufferData(HoveringMesh->GetMeshID(), true);
		BufferMgr.BindVertexBufferObjectByID(HoveringMesh->GetMeshID(), HoveringMesh->GetPosition().data(), HoveringMesh->GetPosition().size(),
			HoveringMesh->GetColor().data(), HoveringMesh->GetColor().size(), nullptr, 0);
		BufferMgr.BindElementBufferObjectByID(HoveringMesh->GetMeshID(), HoveringMesh->GetIndex().data(), HoveringMesh->GetIndex().size());
		FeatureCurveMgr.SetHoveringConstraintPointDirty(false);
	}

	glfwGetCursorPos(window, &mgl.x, &mgl.y);

	int ww, wh;
	glfwGetWindowSize(window, &ww, &wh);

	glfwGetFramebufferSize(window, &FrameBufferWidth, &FrameBufferHeight);

	double sx = mgl.x * (static_cast<double>(FrameBufferWidth) / static_cast<double>(ww));
	double sy = mgl.y * (static_cast<double>(FrameBufferHeight) / static_cast<double>(wh));

	mgl = transformMouseToGL(sx, sy, FrameBufferWidth, FrameBufferHeight);
	glm::vec3 point = RayfromMouse(mgl, projection, view);
	glm::vec3 ray = camera[0] + point * 500.0f;

	float t;

	glm::vec3 dir = ray - camera[0];
	t = -camera[0].y / dir.y;
	glm::vec3 hit = camera[0] + dir * t;

	glm::vec3 Pos = glm::vec3(hit.x, 0.0f, hit.z);

	FeatureCurveMgr.HoverPressedShift(Pos);

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
		if (yoffset > 0 && CameraSpeed < 10.0f) CameraSpeed += 0.001f;
		else if (yoffset < 0 && CameraSpeed > 0.001f) CameraSpeed -= 0.001f;
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

void CurveManagerViewer() {

	CurveManagerView CurveView = FeatureCurveMgr.GetCurveManagerView();

	if (CurveView.ConstraintPointPanelOpen) {

		DrawConstraintPointPanel(CurveView);

	}

}

void DrawConstraintPointPanel(const CurveManagerView& CurveView) {

	ImGuiIO& io = ImGui::GetIO();

	ImVec2 pos = ImVec2(20.0f, io.DisplaySize.y / 4.0f);

	auto& cp = FeatureCurveMgr.GetFeatureCurve(CurveView.SelectedCurveID)->GetConstraintPoint(CurveView.SelectedConstraintPointID);

	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.9f);
	ImGui::SetNextWindowSize(ImVec2(170.0f, io.DisplaySize.y / 2.0f), ImGuiCond_Always);
	
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar;

	Constraints InputConstraints = cp.GetConstraints();
	ConstraintMask InputConstraintMask = cp.GetConstraintMask();

	ImGui::Begin("Constraint Point", nullptr, flags);

	bool elevation = HasMask(InputConstraintMask, ConstraintMask::Elevation);
	if (ImGui::Checkbox("Elevation", &elevation)) {
		if (elevation) InputConstraintMask |= ConstraintMask::Elevation;
		else InputConstraintMask = static_cast<ConstraintMask>(static_cast<int>(InputConstraintMask) & ~static_cast<int>(ConstraintMask::Elevation));
	}

	if (elevation) {
		ImGui::InputFloat("h", &InputConstraints.h, 0.0f, 1.0f);
		ImGui::InputFloat("r", &InputConstraints.r, 0.0f, 1.0f);
	}

	bool gradient = HasMask(InputConstraintMask, ConstraintMask::Gradient);
	if (ImGui::Checkbox("Gradient", &gradient)) {
		if (gradient) InputConstraintMask |= ConstraintMask::Gradient;
		else InputConstraintMask = static_cast<ConstraintMask>(static_cast<int>(InputConstraintMask) & ~static_cast<int>(ConstraintMask::Gradient));
	}

	if (gradient) {
		ImGui::InputFloat("a", &InputConstraints.a, 0.0f, 1.0f);
		ImGui::InputFloat("b", &InputConstraints.b, 0.0f, 1.0f);
		ImGui::InputFloat("theta", &InputConstraints.theta, -90.0f, 90.0f);
		ImGui::InputFloat("phi", &InputConstraints.phi, -90.0f, 90.0f);
	}

	bool noise = HasMask(InputConstraintMask, ConstraintMask::Noise);
	if (ImGui::Checkbox("Noise", &noise)) {
		if (noise) InputConstraintMask |= ConstraintMask::Noise;
		else InputConstraintMask = static_cast<ConstraintMask>(static_cast<int>(InputConstraintMask) & ~static_cast<int>(ConstraintMask::Noise));
	}

	if (noise) {
		ImGui::InputFloat("Amplitude", &InputConstraints.Amplitude, 0.0f, 1.0f);
		ImGui::InputFloat("Roughness", &InputConstraints.Roughness, 0.0f, 1.0f);
	}

	cp.SetConstraints(InputConstraints);
	cp.SetConstraintMask(InputConstraintMask);

	ImGui::End();

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
			if (LightSource == glm::vec3(0.0f, 0.0f, 0.0f))LightSource = glm::vec3(0.5f, 10.0f, 0.5f);
			else LightSource = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		if (ImGui::Button("Rasterize", ImVec2(-FLT_MIN, 30))) {
			RasterizerMgr.SetCurves(FeatureCurveMgr.ExtractCurveData());
			RasterizerMgr.Initialize(HeightMapU, HeightMapV);
			RasterizerMgr.BuildPolyline();
			//RasterizerMgr.PrintPolylines();
			RasterizerMgr.InterpolateCurves();
			//RasterizerMgr.PrintPolylineMasks();
			RasterizerMgr.BuildQuads();
			//RasterizerMgr.PrintQuads();
			RasterizerMgr.BuildConstraintMaps();
			// x방향이 col임
			//RasterizerMgr.PrintQuads();
			std::vector<uint8_t> constraintMask = RasterizerMgr.GetMaps().ConstraintMaskMap;
			ExportConstraintMaskImage("ConstraintMask.png", constraintMask);

		}

		


		if (ImGui::InputInt("GradientIteration", &GradientIteration, 1, 100));
		if (ImGui::InputInt("ElevationIteration", &ElevationIteration, 1, 100));
		if (ImGui::Button("Diffusion", ImVec2(-FLT_MIN, 30))) {

			Maps ConstraintMap = RasterizerMgr.GetMaps();

			ExportGradientImage("RasterizedGradientRGB.png", ConstraintMap.Gradients, true);
			ExportHeightmapImage("RasterizedElevationGray.png", ConstraintMap.ElevationMap);
			ExportConstraintMaskImage("RasterizedConstraintMask.png", ConstraintMap.ConstraintMaskMap);

			BufferMgr.UploadElevationTexture(HeightMapU, HeightMapV, ConstraintMap.ElevationMap.data());
			BufferMgr.UploadGradientTexture(HeightMapU, HeightMapV, ConstraintMap.Gradients);
			BufferMgr.UploadNoiseTexture(HeightMapU, HeightMapV, ConstraintMap.NoiseMap);
			BufferMgr.UploadConstraintMaskTexture(HeightMapU, HeightMapV, ConstraintMap.ConstraintMaskMap.data());
			//BufferMgr.UploadDebugTexture(HeightMapU, HeightMapV);

			BufferMgr.CreateSSBO();

			// 단일 그리드로 반복시 매우 많은 횟수를 반복해야 함. 256x256 기준 5000회 이상
			// Diffuse Gradient
			for (int i = 0; i < GradientIteration; i++) {
				
				BufferMgr.BindGradientTexture();
				BufferMgr.BindConstraintMaskTexture();
				ShaderMgr.FindComputeProgram(ComputeType::Gradient).Use();
				glDispatchCompute((HeightMapU + 15) / 16, (HeightMapV + 15) / 16, 1);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
				BufferMgr.SwapGradient();
			}
			ExportGradientImage("DiffusedGradientRGB.png", BufferMgr.ReadbackGradientTexture(HeightMapU, HeightMapV), true);
			DiffuseMgr.SetGradientMap(BufferMgr.ReadbackGradientTexture(HeightMapU, HeightMapV));
			DiffuseMgr.NormalizeGradients();
			BufferMgr.UploadGradientTexture(HeightMapU, HeightMapV, DiffuseMgr.GetGradientMap());
			BufferMgr.ResetGradientPingPong();
			BufferMgr.BindGradientReadOnly();


			/*for (const auto& grad : DiffuseMgr.GetGradientMap()){
				if ((grad.x != 0 || grad.y != 0) && grad.z != 0) cout << "not zero grad: " << grad.x << ", " << grad.y << ", " << grad.z << endl;
			}*/

			
			// Diffuse Elevation
			//BufferMgr.UploadGradientTexture(1024, 1024, DiffuseMgr.GetGradientMap());
			//BufferMgr.BindGradientTexture();
			for (int asd = 0; asd < ElevationIteration; asd++) {
				//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
				BufferMgr.BindDbgTexture();
				BufferMgr.BindElevationTexture();
				BufferMgr.BindNoiseTexture();
				BufferMgr.BindGradientReadOnly();
				BufferMgr.BindConstraintMaskTexture();
				ShaderMgr.FindComputeProgram(ComputeType::Elevation).Use();
				//glm::ivec4 BorderPixels2 = RasterizerMgr.GetBorderPixels2();
				//glm::ivec2 DebugPixel0 = glm::ivec2(BorderPixels2.x, BorderPixels2.y);
				//glm::ivec2 DebugPixel1 = glm::ivec2(BorderPixels2.z, BorderPixels2.w);
				
				//BufferMgr.AskDebugPixel2(ShaderMgr.FindComputeProgram(ComputeType::Elevation).Program, DebugPixel0, DebugPixel1);
				glDispatchCompute((HeightMapU + 15) / 16, (HeightMapV + 15) / 16, 1);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
				//ExportDebugData(BufferMgr.ReadbackDebugTexture(HeightMapU, HeightMapV), asd + 1);
				//BufferMgr.ReadPrintSSBO();
				BufferMgr.SwapElevation();
				BufferMgr.SwapNoise();

			}

			

			//BufferMgr.UnbindElevationTexture();

			DiffuseMgr.SetElevationMap(BufferMgr.ReadbackElevationTexture(HeightMapU, HeightMapV));
			DiffuseMgr.SetNoiseMap(BufferMgr.ReadbackNoiseTexture(HeightMapU, HeightMapV));

			DiffuseMgr.PackMaps();
			
			BufferMgr.UploadDebugTextures(DiffuseMgr.GetPackedMapRGBA(), DiffuseMgr.GetPackedMapRG());

			heightmap.SetHeight(DiffuseMgr.GetElevationMap());
			UpdateSplineSurface();

			ExportHeightmapImage("DiffusedHeightmapGray.png", heightmap.GetHeightMap());
			
		}

		if(ImGui::Button("Export Constraint Maps", ImVec2(-FLT_MIN, 30))) {
			
			ExportGradientImage("gradient.png", BufferMgr.ReadbackGradientTexture(HeightMapU, HeightMapV), true);
			ExportHeightmapImage("heightmap.png", heightmap.GetHeightMap());
			ExportConstraintMaskImage("constraintmask.png", RasterizerMgr.GetMaps().ConstraintMaskMap);
			ExportGradientText("gradient.txt", DiffuseMgr.GetGradientMap());
			//ExportDiffusedGradientDot("dt.txt", DiffuseMgr.GetGradientMap());
			cout << "Exported constraint maps" << endl;
		}

		ImGui::Text("Debug Modes");
		ImGui::Separator();
		if (ImGui::Button("DebugNone", ImVec2(-FLT_MIN, 30))) {
			CurrentDebugMode = DebugMode::None;
			DebugOverlayAlpha = 0.0f;
		}
		if (ImGui::Button("DebugElevation", ImVec2(-FLT_MIN, 30))) {
			CurrentDebugMode = DebugMode::Elevation;
			DebugOverlayAlpha = 1.0f;
		}
		if (ImGui::Button("DebugNormXY", ImVec2(-FLT_MIN, 30))) {
			CurrentDebugMode = DebugMode::NormXY;
			DebugOverlayAlpha = 1.0f;
		}
		if (ImGui::Button("DebugNormZ", ImVec2(-FLT_MIN, 30))) {
			CurrentDebugMode = DebugMode::NormZ;
			DebugOverlayAlpha = 1.0f;
		}
		if (ImGui::Button("DebugAmplitude", ImVec2(-FLT_MIN, 30))) {
			CurrentDebugMode = DebugMode::Amplitude;
			DebugOverlayAlpha = 1.0f;
		}
		if (ImGui::Button("DebugRoughness", ImVec2(-FLT_MIN, 30))) {
			CurrentDebugMode = DebugMode::Roughness;
			DebugOverlayAlpha = 1.0f;
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
			ExportHeightMap("heightmap.txt");
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

	for (int Row = 0; Row < HeightMapV; Row++) {
		for (int Col = 0; Col < HeightMapU; Col++) {
			//uint8_t mask = RasterizerMgr.GetMaps().ConstraintMaskMap[Row * HeightMapU + Col];
			float height = heightmap(Col, Row);
			file.write(reinterpret_cast<const char*>(&height), sizeof(float));
			//file << (int)mask << " ";
		}
		//file << "\n";
	}

	//for (int x = 0; x < sampleCount; x++) {
	//	for (int z = 0; z < sampleCount; z++) {
	//		float height = SurfacePoints[x * sampleCount + z].y;
	//		height = glm::clamp(height, 0.0f, 1.0f);
	//		uint16_t height16 = static_cast<uint16_t>(height * 65535.0f);
	//		file.write(reinterpret_cast<const char*>(&height16), sizeof(height16));
	//	}
	//}

	file.close();
}

void ExportGradientImage(const char* FileName, const std::vector<glm::vec3>& Map, bool ExportNorm) {

	std::vector<uint8_t> image(HeightMapU * HeightMapV * 3);

	for(int Row = 0; Row < HeightMapV; Row++) {
		for(int Col = 0; Col < HeightMapU; Col++) {
			float x = Map[Row * HeightMapU + Col].x;
			float y = Map[Row * HeightMapU + Col].y;
			float norm = Map[Row * HeightMapU + Col].z;

			uint8_t r = (uint8_t)((x * 0.5f + 0.5f) * 255.0f);
			uint8_t g = (uint8_t)((y * 0.5f + 0.5f) * 255.0f);
 			uint8_t b = 128;
			if(ExportNorm) b = (uint8_t)(norm * 255.0f);

			image[(Row * HeightMapU + Col) * 3 + 0] = 0;
			image[(Row * HeightMapU + Col) * 3 + 1] = 0;
			image[(Row * HeightMapU + Col) * 3 + 2] = b;
		}	
	}
	stbi_write_png(FileName, HeightMapU, HeightMapV, 3, image.data(), HeightMapU * 3);

}

void ExportGradientText(const char* FileName, const std::vector<glm::vec3>& Map) {
	ofstream file(FileName);

	for (int Row = 0; Row < HeightMapV; Row++) {
		for (int Col = 0; Col < HeightMapU; Col++) {
			float Gradient = Map[Row * HeightMapU + Col].z;

			file << Gradient << " ";
		}
		file << "\n";
	}

	file.close();
}

void ExportHeightmapImage(const char* FileName, const std::vector<float>& Map) {
	std::vector<uint8_t> image(HeightMapU * HeightMapV);
	for (int Row = 0; Row < HeightMapV; Row++) {
		for (int Col = 0; Col < HeightMapU; Col++) {
			float h = Map[Row * HeightMapU + Col];
			uint8_t value = (uint8_t)(h * 255.0f);
			image[Row * HeightMapU + Col] = value;
		}
	}
	stbi_write_png(FileName, HeightMapU, HeightMapV, 1, image.data(), HeightMapU);
}

void ExportConstraintMaskImage(const char* FileName, const std::vector<uint8_t>& Map) {
	std::vector<uint8_t> image(HeightMapU * HeightMapV);
	for (int Row = 0; Row < HeightMapV; Row++) {
		for (int Col = 0; Col < HeightMapU; Col++) {
			uint8_t mask = Map[Row * HeightMapU + Col];
			if (mask == 0) mask = 0;
			if (mask == 1) mask = 85;
			if (mask == 2) mask = 170;
			image[Row * HeightMapU + Col] = mask;
		}
	}
	stbi_write_png(FileName, HeightMapU, HeightMapV, 1, image.data(), HeightMapU);
}

void ExportDiffusedGradientDot(const char* FileName, const std::vector<glm::vec3>& Map) {

	static int count = 0;
	std::cout << count << endl;

	std::cout << "U=" << HeightMapU
		<< " V=" << HeightMapV
		<< " MapSize=" << Map.size() << std::endl;

	ofstream file(FileName);
	file.setf(std::ios::fixed);
	file.precision(6);
	for (int Row = 0; Row < HeightMapV; Row++) {
		for (int Col = 0; Col < HeightMapU; Col++) {
			int idx = Row * HeightMapU + Col;
			glm::ivec2 p = ivec2(Col, Row);
			glm::ivec2 Neighbor;
			if (abs(Map[Row * HeightMapU + Col].x) > abs(Map[idx].y)) Neighbor = p + glm::ivec2((Map[idx].x > 0) ? -1 : 1, 0);
			else Neighbor = p + glm::ivec2(0, (Map[idx].y > 0) ? -1 : 1);
			Neighbor = glm::clamp(Neighbor, glm::ivec2(0, 0), glm::ivec2(HeightMapU - 1, HeightMapV - 1));
			glm::ivec2 Diff = p - Neighbor;
			glm::vec2 Direction = Diff;
			glm::vec2 Gradient = glm::vec2(Map[idx].x, Map[idx].y) * Map[idx].z;
			float Delta = glm::dot(Gradient, Direction);
			//if (!std::isfinite(Delta)) Delta = 0.0f;
			//file.write(reinterpret_cast<char*>(&Delta), sizeof(float));
			
			//count++;
			//std::cout << count << endl;
			file << Delta << " ";
		}
		file << "\n";
	}

	file.close();

}

void ExportDiffusedGradientImage(const char* FileName, const std::vector<glm::vec3>& Map) {
	std::vector<uint8_t> image(HeightMapU * HeightMapV * 3);

	for (int Row = 0; Row < HeightMapV; Row++) {
		for (int Col = 0; Col < HeightMapU; Col) {

			glm::ivec2 p = ivec2(Col, Row);
			glm::ivec2 Neighbor;
			if (abs(Map[Row * HeightMapU + Col].x) > abs(Map[Row * HeightMapU + Col].y)) Neighbor = p + glm::ivec2((Map[Row * HeightMapU + Col].x > 0) ? -1 : 1, 0);
			else Neighbor = p + glm::ivec2(0, (Map[Row * HeightMapU + Col].y > 0) ? -1 : 1);
			Neighbor = glm::clamp(Neighbor, glm::ivec2(0, 0), glm::ivec2(HeightMapU - 1, HeightMapV - 1));
			glm::ivec2 Diff = p - Neighbor;
			glm::vec2 Direction = Diff;
			glm::vec2 Gradient = glm::vec2(Map[Row * HeightMapU + Col].x, Map[Row * HeightMapU + Col].y) * Map[Row * HeightMapU + Col].z;
			float Delta = glm::dot(Gradient, Direction);
			uint8_t value = (uint8_t)(Delta * 255.0f);
			image[(Row * HeightMapU + Col) * 3 + 0] = value;
			image[(Row * HeightMapU + Col) * 3 + 1] = value;
			image[(Row * HeightMapU + Col) * 3 + 2] = value;
		}
	}
	stbi_write_png(FileName, HeightMapU, HeightMapV, 3, image.data(), HeightMapU * 3);

}

void ExportDebugData(const std::vector<glm::vec4>& Map, const int Iteration) {

	std::string Filename1 = "OldHeight" + to_string(Iteration) + ".txt";
	std::string Filename2 = "F_N" + to_string(Iteration) + ".txt";
	std::string Filename3 = "F_G" + to_string(Iteration) + ".txt";
	std::string Filename4 = "NewHeight" + to_string(Iteration) + ".txt";
	ofstream file1(Filename1);
	ofstream file2(Filename2);
	ofstream file3(Filename3);
	ofstream file4(Filename4);

	for(int Row = 0; Row < HeightMapV; Row++) {
		for(int Col = 0; Col < HeightMapU; Col++) {
			glm::vec4 data = Map[Row * HeightMapU + Col];

			file1 << data.r << " ";
			file2 << data.g << " ";
			file3 << data.b << " ";
			file4 << data.a << " ";
			
		}
		file1 << "\n";
		file2 << "\n";
		file3 << "\n";
		file4 << "\n";
	}

	file1.close();
	file2.close();
	file3.close();
	file4.close();
}