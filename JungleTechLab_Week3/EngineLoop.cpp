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
#include "PrimitiveComponent.h"

#include <objbase.h>




void FEngineLoop::Init(HINSTANCE hInstance, WNDPROC WndProc)
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) { UE_LOG("Failed to initialize COM library."); }


	//********************로딩창 이미지 출력********************
	 //로딩 동안 보여줄 스플래시. 메인 창은 D3D 스왑체인이 덮어쓰므로 별도 팝업 창에 그린다
		HWND splashWnd = nullptr;
		if (HBITMAP splash = (HBITMAP)LoadImageW(nullptr, L"./Assets/ghoast.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE))
		{
			BITMAP bm; GetObject(splash, sizeof(bm), &bm);
			const int w = GetSystemMetrics(SM_CXSCREEN);   // 화면 전체
			const int h = GetSystemMetrics(SM_CYSCREEN);

			WNDCLASSW splashClass = { 0, DefWindowProcW, 0, 0, 0, 0, 0, 0, 0, L"JungleSplash" };
			RegisterClassW(&splashClass);
			splashWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"JungleSplash", L"",
				WS_POPUP | WS_VISIBLE, 0, 0, w, h, nullptr, nullptr, hInstance, nullptr);

			HDC dc = GetDC(splashWnd);

			// 1) 흰 배경
			RECT full = { 0, 0, w, h };
			FillRect(dc, &full, (HBRUSH)GetStockObject(WHITE_BRUSH));

			// 2) 유령을 비율 유지해서 가운데. 화면 높이의 60% 로 맞춘다 (원하면 비율 조정)
			const int drawH = static_cast<int>(h * 0.6f);
			const int drawW = drawH * bm.bmWidth / bm.bmHeight;
			const int drawX = (w - drawW) / 2;
			const int drawY = (h - drawH) / 2;

			HDC memDC = CreateCompatibleDC(dc);
			SelectObject(memDC, splash);
			SetStretchBltMode(dc, HALFTONE);
			StretchBlt(dc, drawX, drawY, drawW, drawH, memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
			DeleteDC(memDC);
			ReleaseDC(splashWnd, dc);
			DeleteObject(splash);

		}

	
	//********************로딩창 이미지 출력********************

	// Initialize window infos
	WCHAR WindowClass[] = L"JungleWindowClass";
	WCHAR Title[] = L"Ghost Engine";
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };
	RegisterClassW(&wndclass);

	HWND hWnd = CreateWindowExW(
		0, WindowClass, Title, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1920, 1080,
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

	//GraphicsManager = new FGraphicsManager(hWnd);
	//GraphicsManager 초기화
	FGraphicsManager::Get().Initialize(hWnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(FGraphicsManager::Get().GetRenderer()->Device, FGraphicsManager::Get().GetRenderer()->DeviceContext);

	ConsoleWindow& console = ConsoleWindow::GetInstance();
	console.Init("Jungle Console Window", clientWidth);

	// 매니저 할당 및 디바이스 주입	
	FResourceManager::Get().Initialize(FGraphicsManager::Get().GetRenderer()->Device, FGraphicsManager::Get().GetRenderer()->DeviceContext);
	FileManager = new FFileManager(hWnd);

	// 에셋 생성 로직을 ResourceManager에서만 처리
	FResourceManager::Get().InitializeDefaultAssets(&FGraphicsManager::Get());

	FrameTimer = new FFrameTimer(120);
	ViewportClient = new FEditorViewportClient();

	SceneManager = new FSceneManager();
	SceneManager->SetResourceManager(&FResourceManager::Get(), &FGraphicsManager::Get());
	//SceneManager->RequestNewScene();
	SceneManager->RequestNewScene();
	SceneManager->SetHwnd(hWnd);

	//로딩창 제거
	Tick(false);
	if (splashWnd) DestroyWindow(splashWnd);

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
			SceneManager->UpdateGUI({ *FrameTimer, &FGraphicsManager::Get(), ViewportClient, &FResourceManager::Get() ,FileManager });
		}

		FGraphicsManager::Get().UpdateProjectionTransition(deltaTime);
		ViewportClient->Update(deltaTime, FGraphicsManager::Get().GetRenderer()->ViewportInfo, SceneManager, FGraphicsManager::Get().GetPerspectiveRatio(), FResourceManager::Get().IniConfig);
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

			FGraphicsManager::Get().GetRenderer()->OnResize(WindowApplication.PendingWidth, WindowApplication.PendingHeight, viewportWidth, viewportHeight);
			WindowApplication.bPendingResize = false;
		}

		FGraphicsManager::Get().Update(deltaTime);
		FGraphicsManager::Get().Prepare(&ViewportClient->mCamera, ViewportClient->GetViewMode());

		// Show Flag에 따른 렌더 선택 분기
		EEngineShowFlags flags = FResourceManager::Get().IniConfig.GetShowFlags();


		if (HasFlag(flags, EEngineShowFlags::SF_Primitives))
		{
			FGraphicsManager::Get().Render(SceneManager->GetRenderInfos());
		}

		//월드 축. 액터 뒤에 그려서 같은 깊이 버퍼로 가려지게 한다 (기즈모와 달리 깊이를 지우지 않는다)
		if (HasFlag(flags, EEngineShowFlags::SF_WorldAxis))
		{
			FGraphicsManager::Get().DrawWorldAxis();
		}
		if (HasFlag(flags, EEngineShowFlags::SF_Grid))
		{
			FGraphicsManager::Get().DrawGrid(ViewportClient->GetCamera().Transform, FResourceManager::Get().IniConfig.GetGridOffset(), FResourceManager::Get().IniConfig.GetGridRange());
		}
		//Grid

		//강조
		if (auto SelectedActor = SceneManager->GetSelectedActor())
		{
			FRenderInfo clickedRenderInfo;

			if (SelectedActor->GetFirstRenderInfo(clickedRenderInfo))
			{
				FGraphicsManager::Get().RenderHighLight(clickedRenderInfo);
			}

			if (HasFlag(flags, EEngineShowFlags::SF_BoundingBoxes) && SelectedActor->GetRootComponent()->IsA(UPrimitiveComponent::GetClass()))
			{
				FGraphicsManager::Get().DrawAABB(static_cast<UPrimitiveComponent*>(SelectedActor->GetRootComponent())->GetWorldBounds());
			}
		}

		FGraphicsManager::Get().FlushLines();

		// UUID 텍스쳐 랜더링
		if (HasFlag(flags, EEngineShowFlags::SF_UUID))
		{
			if (UWorld* World = SceneManager->GetCurrentWorld())
			{
				for (AActor* Actor : World->GetActors())
				{
					UPrimitiveComponent* RootComponent = static_cast<UPrimitiveComponent*>(Actor->GetRootComponent());
					FGraphicsManager::Get().DrawCurrentUUID(RootComponent, ViewportClient->mCamera.GetUpVector(),
						ViewportClient->mCamera.GetRightVector(), Actor->ObjectID.GUID.ToString());
				}
				FGraphicsManager::Get().FlushUUID(FResourceManager::Get().GetTexture("FontTexture"));
			}
		}

		// Gizmo
		if (HasFlag(flags, EEngineShowFlags::SF_Gizmo))
		{
			FGraphicsManager::Get().GizmoPrepare();
			FGraphicsManager::Get().RenderOverlay(ViewportClient->mGizmo.GetGizmoRenderInfo(&FResourceManager::Get()));
		}

		// NDC Gizmo
		if (HasFlag(flags, EEngineShowFlags::SF_WorldAxis))
		{
			const FMatrix CameraViewRotation = ViewportClient->GetCamera().Transform.Rotation.ToMatrix().Transpose() * FMatrix::UEToDX;
			FGraphicsManager::Get().DrawGizmoNDC(CameraViewRotation);
		}

		//ImGui
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		FGraphicsManager::Get().Display();
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

	FGraphicsManager::Get().Release();
}
