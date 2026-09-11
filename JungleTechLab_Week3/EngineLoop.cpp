#include "EngineLoop.h"

#include <windows.h>

#include "Renderer.h"
#include "WindowApplication.h"
#include "Console.h"
#include "GraphicsManager.h"
#include "ResourceManager.h"
#include "ObjectFactory.h"
#include "Cube.h"
#include "Sphere.h"
#include "Circle.h"
#include "Quad.h"
#include "Triangle.h"
#include "Object.h"
#include "GizmoArrow.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"
#include "Actor.h"
#include "World.h"

#include "Texture.h"
#include "Material.h"
#include "StaticMesh.h"

void FEngineLoop::Init(HINSTANCE hInstance, WNDPROC WndProc)
{
	// Initialize window infos
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };
	RegisterClassW(&wndclass);

	HWND hWnd = CreateWindowExW(
		0,
		WindowClass,
		Title,
		WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 1024,
		nullptr, nullptr, hInstance, nullptr
	);

	// 창을 화면 크기에 맞게 최대화하여 표시
	ShowWindow(hWnd, SW_SHOWMAXIMIZED);
	UpdateWindow(hWnd);

	// 최대화된 후의 실제 클라이언트 크기를 구해 콘솔에 전달
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	RAWINPUTDEVICE rid = {};
	rid.usUsagePage = 0x01;		// Generic Desktop
	rid.usUsage = 0x02;			// Mouse
	rid.dwFlags = 0;		// 포커스 있을 때만 수신
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	GraphicsManager = new FGraphicsManager(hWnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(GraphicsManager->GetRenderer()->Device, GraphicsManager->GetRenderer()->DeviceContext);

	/* Console Window */
	ConsoleWindow& console = ConsoleWindow::GetInstance();
	console.Init("Jungle Console Window", clientWidth);


	ResourceManager = new FResourceManager();
	FileManager = new FFileManager();

	// ==========================================
	// [1] 그래픽스 버퍼 생성 
	// ==========================================
	FBuffer* CubeBuffer = GraphicsManager->CreateBuffer(Cube_vertices, sizeof(Cube_vertices));
	FBuffer* QuadBuffer = GraphicsManager->CreateBuffer(Quad_vertices, sizeof(Quad_vertices));
	FBuffer* SphereBuffer = GraphicsManager->CreateBuffer(Sphere_vertices, sizeof(Sphere_vertices));
	FBuffer* GizmoArrowBuffer = GraphicsManager->CreateBuffer(GizmoArrow_vertices, sizeof(GizmoArrow_vertices));
	FBuffer* CircleBuffer = GraphicsManager->CreateBuffer(Circle_vertices, sizeof(Circle_vertices));
	// GraphicsManager->CreateBuffer(EPrimitive::EP_Triangle, Triangle_vertices, sizeof(Triangle_vertices));


	// ==========================================
	// [2] 텍스처 에셋 생성 및 등록
	// ==========================================
	UTexture* DefaultTexture = FObjectFactory::ConstructObject<UTexture>();
	// DefaultTexture->Resource = GraphicsManager->LoadTextureFromFile("default.png"); // todo -> 이건 기존에 개발했던거 추가하면 될듯
	ResourceManager->RegisterTexture("DefaultTexture", DefaultTexture);

	// ==========================================
	// [3] 머티리얼 에셋 생성 및 등록
	// ==========================================
	UMaterial* DefaultMaterial = FObjectFactory::ConstructObject<UMaterial>();
	DefaultMaterial->TintColor = FVector4(1.0f, 1.0f, 1.0f, 0.0f);
	DefaultMaterial->BaseTexture = ResourceManager->GetTexture("DefaultTexture");
	ResourceManager->RegisterMaterial("DefaultMaterial", DefaultMaterial);

	// ==========================================
	// [4] 스태틱 메쉬 에셋 생성 및 등록
	// ==========================================
	UStaticMesh* CubeMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	CubeMesh->VertexBuffer = CubeBuffer;
	CubeMesh->StaticMaterials.Add(ResourceManager->GetMaterial("DefaultMaterial"));
	// CPU 정점 복사 (배열 크기만큼)
	uint32 VertCount = sizeof(Cube_vertices) / sizeof(FVertexSimple);
	for (uint32 i = 0; i < VertCount; ++i)
		CubeMesh->CPUVertices.emplace_back(Cube_vertices[i]);
	ResourceManager->RegisterStaticMesh("Cube", CubeMesh);

	UStaticMesh* QuadMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	QuadMesh->VertexBuffer = QuadBuffer;
	QuadMesh->StaticMaterials.Add(ResourceManager->GetMaterial("DefaultMaterial"));
	VertCount = sizeof(Quad_vertices) / sizeof(FVertexSimple);
	for (uint32 i = 0; i < VertCount; ++i)
		QuadMesh->CPUVertices.emplace_back(Quad_vertices[i]);
	ResourceManager->RegisterStaticMesh("Quad", QuadMesh);

	UStaticMesh* SphereMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	SphereMesh->VertexBuffer = SphereBuffer;
	SphereMesh->StaticMaterials.Add(ResourceManager->GetMaterial("DefaultMaterial"));
	VertCount = sizeof(Sphere_vertices) / sizeof(FVertexSimple);
	for (uint32 i = 0; i < VertCount; ++i)
		SphereMesh->CPUVertices.emplace_back(Sphere_vertices[i]);
	ResourceManager->RegisterStaticMesh("Sphere", SphereMesh);

	UStaticMesh* GizmoArrowMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	GizmoArrowMesh->VertexBuffer = GizmoArrowBuffer;
	VertCount = sizeof(GizmoArrow_vertices) / sizeof(FVertexSimple);
	for (uint32 i = 0; i < VertCount; ++i)
		GizmoArrowMesh->CPUVertices.emplace_back(GizmoArrow_vertices[i]);
	ResourceManager->RegisterStaticMesh("GizmoArrow", GizmoArrowMesh);


	UStaticMesh* CircleMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	CircleMesh->VertexBuffer = CircleBuffer;
	VertCount = sizeof(Circle_vertices) / sizeof(FVertexSimple);
	for (uint32 i = 0; i < VertCount; ++i)
		CircleMesh->CPUVertices.emplace_back(Circle_vertices[i]);
	ResourceManager->RegisterStaticMesh("Circle", CircleMesh);




	FrameTimer = new FFrameTimer(120);
	ViewportClient = new FEditorViewportClient(); // Todo: cChange to class

	const FVector4 NearTint(1.0f, 0.65f, 0.15f, 0.85f); // 주황 = 가까운 쪽
	const FVector4 FarTint(0.25f, 0.55f, 1.0f, 0.85f); // 파랑 = 먼 쪽

	SceneManager = new FSceneManager();
	FileManager = new FFileManager();

	SceneManager->NewScene();
	//SceneManager->LoadScene("TestScene", *FileManager);
}

void FEngineLoop::Tick(bool bPumpMessages)
{
	if (GInTick) return;
	GInTick = true;

	FrameTimer->StartFrame();
	float deltaTime = FrameTimer->GetDeltaTime();
	ConsoleWindow& console = ConsoleWindow::GetInstance();

	//Input Threads
	{
		WindowApplication.ProcessDeferredEvents();

		//ImGui Input
		{
			SceneManager->UpdateGUI({ *FrameTimer, GraphicsManager, ViewportClient,ResourceManager,FileManager });
		}

		GraphicsManager->UpdateProjectionTransition(deltaTime);
		ViewportClient->Update(deltaTime, GraphicsManager->GetRenderer()->ViewportInfo, SceneManager, GraphicsManager->GetPerspectiveRatio());
	}

	//Physics Threads
	{

	}

	//Game Threads
	{
		// 레이캐스트보다 먼저 돌려야 한다.
		// 여기서 RenderInfos 가 갱신되고, RayCast 가 그걸 읽는다.
		SceneManager->Update(deltaTime);
	}

	//Render Threads
	{
		if (WindowApplication.bPendingResize)
		{
			float viewportWidth = SceneManager->GetPanelWidth();
			float viewportHeight = (1.f - ConsoleWindow::HEIGHT_RATIO) * WindowApplication.PendingHeight;

			GraphicsManager->GetRenderer()->OnResize(WindowApplication.PendingWidth, WindowApplication.PendingHeight, viewportWidth, viewportHeight);
			WindowApplication.bPendingResize = false;
		}

		GraphicsManager->Update(deltaTime);
		GraphicsManager->Prepare(&ViewportClient->mCamera);
		GraphicsManager->Render(SceneManager->GetRenderInfos());
		

		//월드 축. 액터 뒤에 그려서 같은 깊이 버퍼로 가려지게 한다 (기즈모와 달리 깊이를 지우지 않는다)
		GraphicsManager->DrawWorldAxis();
		GraphicsManager->FlushLines();

		//강조
		if (SceneManager->GetSelectedActor())
		{
			FRenderInfo clickedRenderInfo;
			SceneManager->GetSelectedActor()->GetFirstRenderInfo(clickedRenderInfo);
			GraphicsManager->RenderHighLight(clickedRenderInfo);
		}

		// Gizmo
		GraphicsManager->GizmoPrepare();
		GraphicsManager->RenderOverlay(ViewportClient->mGizmo.GetGizmoRenderInfo(ResourceManager));

		//ImGui
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		GraphicsManager->Display();
	}
	SceneManager->ProcessPendingKills();
	FrameTimer->EndFrame();

	GInTick = false;
}

void FEngineLoop::End()
{
	SceneManager->DeleteScene();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	delete FrameTimer;
	delete SceneManager;
	delete FileManager;

	delete GraphicsManager;
}
