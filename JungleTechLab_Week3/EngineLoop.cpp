#include "EngineLoop.h"

#include <windows.h>

#include "Renderer.h"
#include "WindowApplication.h"
#include "Console.h"
#include "GraphicsManager.h"
#include "ResourceManager.h"
#include "ObjectFactory.h"

#include "Object.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"
#include "Actor.h"
#include "World.h"

#include "Texture.h"
#include "Material.h"
#include "StaticMesh.h"

#include <objbase.h>




void FEngineLoop::Init(HINSTANCE hInstance, WNDPROC WndProc)
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) { UE_LOG("Failed to initialize COM library."); }


	// Initialize window infos
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Game Tech Lab";
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };
	RegisterClassW(&wndclass);

	HWND hWnd = CreateWindowExW(
		0, WindowClass, Title, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 600, 1024,
		nullptr, nullptr, hInstance, nullptr
	);

	ShowWindow(hWnd, SW_SHOWMAXIMIZED);
	UpdateWindow(hWnd);

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	RAWINPUTDEVICE rid = {};
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x02;
	rid.dwFlags = 0;
	rid.hwndTarget = hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));

	GraphicsManager = new FGraphicsManager(hWnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(GraphicsManager->GetRenderer()->Device, GraphicsManager->GetRenderer()->DeviceContext);

	ConsoleWindow& console = ConsoleWindow::GetInstance();
	console.Init("Jungle Console Window", clientWidth);

	// 매니저 할당 및 디바이스 주입
	ResourceManager = new FResourceManager();
	ResourceManager->Initialize(GraphicsManager->GetRenderer()->Device, GraphicsManager->GetRenderer()->DeviceContext);
	FileManager = new FFileManager();

	// 에셋 생성 로직을 ResourceManager에서만 처리
	ResourceManager->InitializeDefaultAssets(GraphicsManager);

	FrameTimer = new FFrameTimer(120);
	ViewportClient = new FEditorViewportClient();

	SceneManager = new FSceneManager();
	//SceneManager->RequestNewScene();
	SceneManager->RequestNewScene();
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

			if (SceneManager->GetSelectedActor()->GetFirstRenderInfo(clickedRenderInfo))
			{
				GraphicsManager->RenderHighLight(clickedRenderInfo);
			}
		}

		// Gizmo
		GraphicsManager->GizmoPrepare();
		GraphicsManager->RenderOverlay(ViewportClient->mGizmo.GetGizmoRenderInfo(ResourceManager));

		// UUID 텍스쳐 랜더링
		GraphicsManager->DrawAllUUID(SceneManager->GetRenderInfos(),
			ViewportClient->mCamera.GetUpVector(), ViewportClient->mCamera.GetRightVector());
		GraphicsManager->FlushUUID();

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
