#pragma once
#include <Windows.h>
#include "FrameTimer.h"
#include "FEditorViewportClient.h"
#include "Camera.h"
#include "SceneManager.h"
#include "FileManager.h"
#include "Renderer.h"
#include "World.h"
#include <d3d11.h>

class FGraphicsManager;
class FResourceManager;

class FEngineLoop
{
public:
	FEngineLoop() {}
	~FEngineLoop() {};

	void Init(HINSTANCE hInstance, WNDPROC WndProc);
	void Tick(bool bPumpMessages);
	void End();
private:
	FGraphicsManager* GraphicsManager = nullptr;
	FSceneManager* SceneManager = nullptr;
	FFileManager* FileManager = nullptr;
	FResourceManager* ResourceManager = nullptr;

	FFrameTimer* FrameTimer = nullptr;
	bool GInTick = false;
	FEditorViewportClient* ViewportClient = nullptr;
};

inline FEngineLoop GEngineLoop;
