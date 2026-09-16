#pragma once
#include "Vector.h"

#include <d3d11.h>
#include "World.h"
#include "Camera.h"
#include "RenderInfo.h"
#include "Gizmo.h"
#include "IniConfig.h"
#include "enum.h"

class AActor;
class FSceneManager;

struct FEditorViewportClient
{
public:
	void RayCast(D3D11_VIEWPORT ViewportInfo, UWorld* World, float perspectiveRatio, FIniConfig IniConfig);
	float GetFov() const { return mCamera.mFovDegree; }
	void Update(float deltaTime, D3D11_VIEWPORT ViewportInfo, FSceneManager* sceneManager, float perspectiveRatio, FIniConfig& IniConfig);
	bool IsMouseHit() const { return bMouseHit; }
	EViewModeIndex GetViewMode() const { return ViewMode; }
	void SetViewMode(EViewModeIndex mode) { ViewMode = mode; }
 void Reset();

	static FCamera& GetCamera(){return mCamera; }

	static FCamera mCamera;
	FGizmo mGizmo;

	// todo
	// 다른 곳에서도 AABB처리가 필요하여 일단 뺏음
	// 이건 별도의 namespace에서 처리하는게 좋아보임
	static bool IsRayIntersectAABB(
		const FVector& RayOrigin,
		const FVector& RayDirection,
		const FVector& BoxCenter,
		const FVector& BoxHalfExtent,
		float& HitTimeAABB);

private:
	AActor* HoveredActor = nullptr;

	bool RayIntersectsTriangle( // 두개의 
		const FVector& Origin,
		const FVector& Dir,
		const FVector& V0,
		const FVector& V1,
		const FVector& V2,
		float& OutT, float& OutU, float& OutV);

	void DeprojectScreenToWorld(int32 MouseX, int32 MouseY,
		float ScreenW, float ScreenH, float NearZ, float FarZ,
		FVector& OutNearPoint, FVector& OutFarPoint);

	void DeprojectScreenToWorldForOrtho(int32 MouseX, int32 MouseY,
		float ScreenW, float ScreenH, float NearZ, float FarZ,
		FVector& OutNearPoint, FVector& OutFarPoint);

	void DeprojectScreenToWorldForUnified(int32 MouseX, int32 MouseY,
		float ScreenW, float ScreenH, float NearZ, float FarZ,
		float orthoDistance, float perspectiveRatio,
		FVector& OutNearPoint, FVector& OutFarPoint
	);



	bool bMouseHit = false;
	EViewModeIndex ViewMode = EViewModeIndex::VMI_Unlit;
	// RayCast가 이번 프레임에 쏜 광선. 기즈모 드래그가 같은 광선을 다시 쓴다
	FVector mRayNear;
	FVector mRayFar;
};
