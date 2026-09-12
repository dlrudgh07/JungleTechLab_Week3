#pragma once

#include "Matrix.h"
#include "Enum.h"

#include "TArray.h"
#include "TMap.h"
#include "Renderer.h"
#include "Camera.h"
#include "RenderInfo.h"
#include "Vector.h"
#include "FQuad.h"

class FGraphicsManager
{
public:
	FGraphicsManager(HWND hWindow);
	~FGraphicsManager();

	//void Prepare(const Camera* mCamera);
	void Prepare(const FCamera* mCamera);
	void GizmoPrepare();


	void Render(const TArray<FRenderInfo>& renderInfos);
	//void RenderOverlay(const TArray<FRenderInfo>& renderInfos);
	void RenderOverlay(const TArray<FRenderInfo> renderInfos); //깊이버퍼 초기화
	// FRenderInfo

	void Display();
	void Update(float deltaTime);

	float GetAspect() const { return mAspect; }
	bool GetWireFrame() const { return mbWireFrame; } const
	void SetWireFrame(bool bWireFrame) { mbWireFrame = bWireFrame; }

	bool IsPerspectiveProjection() const;
	void SetPerspectiveProjection(bool bPerspectiveProjection);

	float GetPerspectiveRatio() const { return mProjectionRatio; }
	void SetPerspectiveRatio(float ratio) { mProjectionRatio = FMath::Clamp(ratio, 0.0f, 1.0f); }

	float GetCameraOrthoDistance() const { return mCameraOrthoDistance; }
	void SetCameraOrthoDistance(float distance) { mCameraOrthoDistance = distance; }

	FBuffer* CreateBuffer(FVertexSimple* InputVertices, uint32 InputVerticesSize);
	URenderer* GetRenderer() const;

	//Highlight
	//Line batch
	// 호출 즉시 그리지 않고 배열에 쌓는다. FlushLines()에서 한 번에 그린다.
	void DrawLine(const FVector& start, const FVector& end, const FVector4& color);
	void DrawWorldAxis();
	void FlushLines();


	//UUID
	// 호출 즉시 그리지 않고 배열에 쌓는다. FlushUUID()에서 한 번에 그린다.
	void DrawUUID(const FQuad& quad);
	void DrawAllUUID(const TArray<FRenderInfo> renderInfos, FVector UpVector, FVector RightVector);
	void FlushUUID(UTexture* FontTexture);

	bool GetShowWorldAxis() const { return mbShowWorldAxis; }
	void SetShowWorldAxis(bool bShow) { mbShowWorldAxis = bShow; }

	static FVector GetPrimitiveCenter(EPrimitive type);
	static FVector GetPrimitiveHalfExtent(EPrimitive type);
	void RenderHighLight(const FRenderInfo& RI);

	// Projection ratio smoothing
	void StartProjectionTransition(bool orthographic);
	bool IsOrthographicTarget() const;
	void UpdateProjectionTransition(float deltaTime);

private:
	URenderer* mRenderer = nullptr;
	FMatrix mViewProjectionMatrix{};
	FMatrix mViewOrthogonalProjectionMatrix{};
	FMatrix mViewUnifiedProjectionMatrix{};

	// Prepare에서 갱신. 하이라이트 두께의 픽셀 → 월드 환산에 쓴다
	FVector mCameraLocation{};
	FVector mCameraForward{};
	float mCameraFovDegree = 60.0f;
	float mCameraOrthoDistance = 10.0f;

	// Graphics config
	// 이번 프레임에 쌓인 선분. 정점 2개가 선분 하나
	TArray<FVertexSimple> mLineVertices;
	TArray<FVertexSimple> mUUIDVertices;

	bool mbWireFrame = false;
	bool mbPerspectiveProjection = false;
	bool mbShowWorldAxis = true;
	float mAspect = 0;
	float mProjectionRatio = 0; // 0.0f ~ 1.0f, 0이면 직교, 1이면 원근, 그 사이면 혼합

	// Projection ratio smoothing
	float mProjectionStartRatio = 1.0f;
	float mProjectionTargetRatio = 1.0f;
	float mProjectionElapsed = 0.0f;
	float mProjectionDuration = 1.0f;
	bool mbProjectionTransitioning = false;
};
