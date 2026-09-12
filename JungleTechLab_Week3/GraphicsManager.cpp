#include "GraphicsManager.h"

#include "Renderer.h"
#include "Camera.h"
#include "Console.h"
#include "StaticMesh.h"
#include "Texture.h"
#include "FQuad.h"

// 선분 하나당 정점 2개. 축 6개 + 앞으로 붙을 그리드까지 감당할 만큼 잡아둔다
static constexpr uint32 LINE_VERTEX_CAPACITY = 8192;
static constexpr uint32 UUID_VERTEX_CAPACITY = 10; // 초기에할당한 크기이다 용량이 꽉차면 2배로 재할당

FGraphicsManager::FGraphicsManager(HWND hWindow)
	: mbWireFrame(false)
	, mbPerspectiveProjection(true)
	, mProjectionRatio(1.0f)
{
	mRenderer = new URenderer;
	mRenderer->Create(hWindow);
	mRenderer->CreateShader();
	mRenderer->CreateConstantBuffer();
	mRenderer->CreateLineVertexBuffer(LINE_VERTEX_CAPACITY);
	mRenderer->CreateUUIDVertexBuffer(UUID_VERTEX_CAPACITY);
	mRenderer->CreateUUIDSampleState();

	mAspect = mRenderer->ViewportInfo.Width / mRenderer->ViewportInfo.Height;
}

FGraphicsManager::~FGraphicsManager()
{
	mRenderer->ReleaseLineVertexBuffer();
	mRenderer->ReleaseConstantBuffer();
	mRenderer->ReleaseUUIDVertexBuffer();
	mRenderer->ReleaseShader();
	mRenderer->Release();

	delete mRenderer;
}

void FGraphicsManager::Prepare(const FCamera* mCamera)
{
	mRenderer->Prepare(mbWireFrame);
	mRenderer->PrepareShader();

	// Cache view and projection matrices for rendering
	const float nearZ = 0.1f;
	const float farZ = 100.0f;

	float d = mCamera->mOrthoDistance;

	FMatrix view = mCamera->GetViewMatrix();
	FMatrix projection_u_p = mCamera->GetUnifiedProjectionMatrix(mAspect, mCamera->mFovDegree, d, nearZ, farZ, 1.0f);
	FMatrix projection_u_o = mCamera->GetUnifiedProjectionMatrix(mAspect, mCamera->mFovDegree, d, nearZ, farZ, 0.0f);
	FMatrix projection_u = mCamera->GetUnifiedProjectionMatrix(mAspect, mCamera->mFovDegree, d, nearZ, farZ, mProjectionRatio);

	//mViewProjectionMatrix = view * mCamera->GetProjectionMatrix(mAspect, mCamera->mFovDegree, nearZ, farZ);
	mViewProjectionMatrix = view * projection_u_p;

	float orthoHeight = mCamera->mOrthoHeight;
	float orthoWidth = orthoHeight * mAspect;
	//mViewOrthogonalProjectionMatrix = view * mCamera->GetOrthographicMatrix(orthoWidth, orthoHeight, nearZ, farZ);
	mViewOrthogonalProjectionMatrix = view * projection_u_o;
	mViewUnifiedProjectionMatrix = view * projection_u;

	// 하이라이트 두께를 화면 픽셀 기준으로 환산할 때 쓴다
	mCameraLocation = mCamera->Transform.Location;
	mCameraForward = mCamera->GetForwardVector();
	mCameraFovDegree = mCamera->mFovDegree;
	mCameraOrthoDistance = mCamera->mOrthoDistance;

	// 그리는 순서가 중요하다: 가까운 것을 먼저, 먼 것을 나중에.
	// 깊이 테스트가 켜져 있으면 나중에 그린 FarCube 가 깊이 비교에서 탈락해
	// NearCube(주황)가 앞에 남고, 꺼져 있으면 FarCube(파랑)가 그 위를 덮어쓴다.
	//mRenderer->UpdateConstantViewProjection(viewProjection);
}
void FGraphicsManager::GizmoPrepare()
{
	mRenderer->RSUpdateState();

}
void FGraphicsManager::Render(const TArray<FRenderInfo> renderInfos)
{
	FMatrix viewProjection;
	//if (mbPerspectiveProjection)
	//{
	//	viewProjection = mViewProjectionMatrix;
	//}
	//else
	//{
	//	viewProjection = mViewOrthogonalProjectionMatrix;
	//}

	viewProjection = mViewUnifiedProjectionMatrix;

	for (const FRenderInfo& renderInfo : renderInfos)
	{
		//mRenderer->UpdateConstant(renderInfo.WorldTransformMatrix, mViewProjectionMatrix, renderInfo.Color);
		// todo: 조명처리할게 아니라면, mvp 처리는 cpu에서 하고 넘기는더 효율적이다
		mRenderer->UpdateConstant(renderInfo.WorldTransformMatrix, viewProjection, renderInfo.Color);

		if (renderInfo.VertexBuffer == nullptr || renderInfo.VertexBuffer->VertexBuffer == nullptr)
			continue;


		if (renderInfo.BaseTexture != nullptr && renderInfo.BaseTexture->Resource != nullptr)
		{
			mRenderer->BindTexture(renderInfo.BaseTexture->Resource->SRV.Get());
		}
		else
		{
			// 텍스처가 없으면 렌더러에 내장된 1x1 화이트 텍스처를 꽂아 정점 색상을 보존
			mRenderer->BindTexture(mRenderer->DefaultWhiteTextureSRV);
		}

		mRenderer->RenderPrimitive(renderInfo.VertexBuffer->VertexBuffer.Get(), renderInfo.VertexBuffer->NumVertices);
	}
}






void FGraphicsManager::DrawLine(const FVector& start, const FVector& end, const FVector4& color)
{
	// 월드 좌표 그대로 넣는다. 그래서 그릴 때 World 행렬이 단위행렬이다
	mLineVertices.Add({ start.x, start.y, start.z, color.x, color.y, color.z, color.w });
	mLineVertices.Add({ end.x,   end.y,   end.z,   color.x, color.y, color.z, color.w });
}

void FGraphicsManager::DrawWorldAxis()
{
	if (!mbShowWorldAxis) return;

	// far plane이 100이라 그 안쪽으로 잡아야 잘리지 않는다
	constexpr float AXIS_LENGTH = 50.0f;
	// 세 축이 원점에서 정확히 겹치면 깊이 다툼이 생긴다. 눈에 안 띌 만큼만 띄운다
	constexpr float AXIS_ORIGIN_GAP = 0.01f;
	// 음의 방향은 어둡게 깔아 +쪽과 구분한다 (언리얼 에디터와 같은 처리)
	constexpr float NEGATIVE_DIM = 0.25f;

	const FVector axisDirections[3] =
	{
		FVector(1.0f, 0.0f, 0.0f),
		FVector(0.0f, 1.0f, 0.0f),
		FVector(0.0f, 0.0f, 1.0f),
	};
	const FVector4 axisColors[3] =
	{
		FVector4(1.0f, 0.0f, 0.0f, 1.0f),   // X = 빨강
		FVector4(0.0f, 1.0f, 0.0f, 1.0f),   // Y = 초록
		FVector4(0.0f, 0.4f, 1.0f, 1.0f),   // Z = 파랑
	};

	for (int32 i = 0; i < 3; ++i)
	{
		const FVector& direction = axisDirections[i];
		const FVector4& color = axisColors[i];
		const FVector4 dimColor(
			color.x * NEGATIVE_DIM,
			color.y * NEGATIVE_DIM,
			color.z * NEGATIVE_DIM,
			color.w);

		DrawLine(direction * AXIS_ORIGIN_GAP, direction * AXIS_LENGTH, color);
		DrawLine(direction * -AXIS_ORIGIN_GAP, direction * -AXIS_LENGTH, dimColor);
	}
}

void FGraphicsManager::FlushLines()
{
	if (mLineVertices.Num() == 0) return;

	// 선분 좌표가 이미 월드 공간이라 World는 단위행렬.
	// Tint.a = 0 이면 셰이더의 lerp가 정점 색을 그대로 통과시킨다
	//if (mbPerspectiveProjection)
	//{
	//	mRenderer->UpdateConstant(FMatrix::Identity, mViewProjectionMatrix, FVector4(0, 0, 0, 0));
	//}
	//else
	//{
	//	mRenderer->UpdateConstant(FMatrix::Identity, mViewOrthogonalProjectionMatrix, FVector4(0, 0, 0, 0));
	//}
	mRenderer->UpdateConstant(FMatrix::Identity, mViewUnifiedProjectionMatrix, FVector4(0, 0, 0, 0));
	mRenderer->RenderLines(&mLineVertices[0], mLineVertices.Num());

	// 안 비우면 매 프레임 누적돼 버퍼가 넘친다. 용량은 유지한 채 개수만 0으로
	mLineVertices.Reset(LINE_VERTEX_CAPACITY);
}


void FGraphicsManager::DrawUUID(const FQuad& quad)
{
	// 월드 좌표 그대로 넣는다. 그래서 그릴 때 World 행렬이 단위행렬이다
	mUUIDVertices.Add({ quad.LeftDown.x , quad.LeftDown.y, quad.LeftDown.z, 1.0f,1.0f,1.0f,1.0f, quad.u[0], quad.v[1]}); // 좌측하단
	mUUIDVertices.Add({ quad.RightUp.x, quad.RightUp.y, quad.RightUp.z, 1.0f,1.0f,1.0f,1.0f, quad.u[1], quad.v[0] }); // 우측상단
	mUUIDVertices.Add({ quad.RightDown.x, quad.RightDown.y, quad.RightDown.z, 1.0f,1.0f,1.0f,1.0f, quad.u[1], quad.v[1] }); // 우측하단
	mUUIDVertices.Add({ quad.RightUp.x, quad.RightUp.y, quad.RightUp.z, 1.0f,1.0f,1.0f,1.0f, quad.u[1], quad.v[0] }); // 우측상단
	mUUIDVertices.Add({ quad.LeftDown.x, quad.LeftDown.y, quad.LeftDown.z, 1.0f,1.0f,1.0f,1.0f, quad.u[0], quad.v[1] }); // 좌측하단
	mUUIDVertices.Add({ quad.LeftUp.x, quad.LeftUp.y, quad.LeftUp.z, 1.0f,1.0f,1.0f,1.0f, quad.u[0], quad.v[0] }); // 좌측상단
}

void FGraphicsManager::DrawAllUUID(const TArray<FRenderInfo> renderInfos, FVector UpVector, FVector RightVector)
{
	for (const FRenderInfo& renderInfo : renderInfos)
	{
		FMatrix CurrentWorldMatrix = renderInfo.WorldTransformMatrix;

		FString UUID = FString("UID:").Append(renderInfo.ObejctID.GUID.ToString());
		FQuad Quad;

		const float UpLength = 0.12f;
		const float RightLength = 0.12f;
		const float WordOffset = 0.09f;
		const uint32 CellLine = 16;
		const float LocalOffst = (1.0f / CellLine);

		FVector ModelVector = CurrentWorldMatrix.TransformPosition(FVector(0, 0, 0));
		FVector Origin = ModelVector - (RightVector * WordOffset * (UUID.Len() * 0.5f));

		Origin.z += 1.0f;

		for (int i = 0; i < UUID.Len(); i++)
		{
			unsigned char CurrentAsciiCode = UUID.At(i);

			FVector Cursor = Origin + RightVector * (WordOffset * i);

			Quad.LeftUp = Cursor + UpVector * UpLength;
			Quad.RightUp = Cursor + RightVector * RightLength + UpVector * UpLength;
			Quad.LeftDown = Cursor;
			Quad.RightDown = Cursor + RightVector * RightLength;

			Quad.u[0] = (0 * LocalOffst) + LocalOffst * (CurrentAsciiCode % CellLine);  // 좌측
			Quad.u[1] = (1 * LocalOffst) + LocalOffst * (CurrentAsciiCode % CellLine);  // 우측

			Quad.v[0] = (0 * LocalOffst) + LocalOffst * (CurrentAsciiCode / CellLine);  // 상단
			Quad.v[1] = (1 * LocalOffst) + LocalOffst * (CurrentAsciiCode / CellLine);  // 하단
			DrawUUID(Quad);
		}
	}
}

void FGraphicsManager::FlushUUID()
{
	if (mUUIDVertices.Num() == 0) return;

	mRenderer->UpdateConstant(FMatrix::Identity, mViewUnifiedProjectionMatrix, FVector4(0, 0, 0, 0));
	//mRenderer->UpdateConstant(renderInfo.WorldTransformMatrix, viewProjection, FVector4(1, 1, 0, 0));

	mRenderer->PrepareTextureShader();
	mRenderer->RenderUUID(&mUUIDVertices[0], mUUIDVertices.Num());
	mRenderer->PrepareShader();

	// 안 비우면 매 프레임 누적돼 버퍼가 넘친다. 용량은 유지한 채 개수만 0으로
	mUUIDVertices.Reset(UUID_VERTEX_CAPACITY); // 나중에 수정해야됨 UUID용으로
}

void FGraphicsManager::RenderOverlay(const TArray<FRenderInfo> renderInfos) //깊이버퍼 초기화
{
	mRenderer->ClearDepth();
	Render(renderInfos);
}
/*
void GraphicsManager::Render(FTransform worldTransformMatrix, EPrimitive ePrimitive)
{
	mRenderer->UpdateConstant(worldTransformMatrix.MakeMatrix(), mViewProjectionMatrix);

	FBuffer vertexBuffer = mBufferMap[ePrimitive];
	mRenderer->RenderPrimitive(vertexBuffer.Buffer, vertexBuffer.SourceNum);
}
*/

void FGraphicsManager::Display()
{
	mRenderer->SwapBuffer();
}

void FGraphicsManager::Update(float deltaTime)
{
	mAspect = mRenderer->ViewportInfo.Width / mRenderer->ViewportInfo.Height;
}

bool FGraphicsManager::IsPerspectiveProjection() const
{
	return mbPerspectiveProjection;
}

void FGraphicsManager::SetPerspectiveProjection(bool bPerspectiveProjection)
{
	mbPerspectiveProjection = bPerspectiveProjection;
}

FBuffer* FGraphicsManager::CreateBuffer(FVertexSimple* InputVertices, uint32 InputVerticesSize)
{
	ID3D11Buffer* rawBuffer = mRenderer->CreateVertexBuffer(InputVertices, InputVerticesSize);
	UINT numVertices = static_cast<UINT>(InputVerticesSize / sizeof(FVertexSimple));

	FBuffer* newBuffer = new FBuffer();
	newBuffer->VertexBuffer = rawBuffer;
	newBuffer->NumVertices = numVertices;

	return newBuffer;
}

URenderer* FGraphicsManager::GetRenderer() const
{
	assert(mRenderer != nullptr);

	return mRenderer;
}


// 테두리가 화면에서 차지할 두께(픽셀). 물체 크기와 카메라 거리 어느 쪽에도 영향받지 않는다.
static constexpr float OUTLINE_PIXELS = 3.0f;

// 월드 공간 반지름이 worldHalfExtent인 축을 worldThickness 만큼 키우는 배율
static float GetOutlineAxisScale(float worldHalfExtent, float worldThickness)
{
	if (worldHalfExtent <= SMALL_NUMBER)
	{
		return 1.0f;   // 납작하게 눌린 축은 건드리지 않는다. 안 그러면 배율이 발산한다
	}

	return 1.0f + worldThickness / worldHalfExtent;
}

void FGraphicsManager::RenderHighLight(const FRenderInfo& RI)
{
	if (RI.VertexBuffer == nullptr || RI.VertexBuffer->VertexBuffer == nullptr)
		return;

	// EPrimitive 하드코딩을 제거하고 RI에서 정보를 가져옴
	const FVector Center = RI.BoundsCenter;
	const FVector HalfExtent = RI.BoundsHalfExtent;

	const FVector ObjectLocation = RI.WorldTransformMatrix.TransformPosition(Center);
	const float Depth = FVector::dot(ObjectLocation - mCameraLocation, mCameraForward);
	const float TanHalfFov = tanf(FMath::DegreesToRadians(mCameraFovDegree * 0.5f));
	const float effectiveDepth = FMath::Max(
		(1.0f - mProjectionRatio) * mCameraOrthoDistance + mProjectionRatio * Depth
		, 0.01f);

	const float H = 2.0f * effectiveDepth * TanHalfFov;
	const float WorldThickness = OUTLINE_PIXELS * H / mRenderer->ViewportInfo.Height;

	const FVector WorldScale(
		RI.WorldTransformMatrix.GetUnitAxis(EAxis::X).Length(),
		RI.WorldTransformMatrix.GetUnitAxis(EAxis::Y).Length(),
		RI.WorldTransformMatrix.GetUnitAxis(EAxis::Z).Length());

	FVector OutlineScale = {
		GetOutlineAxisScale(HalfExtent.x * WorldScale.x, WorldThickness),
		GetOutlineAxisScale(HalfExtent.y * WorldScale.y, WorldThickness),
		GetOutlineAxisScale(HalfExtent.z * WorldScale.z, WorldThickness) };

	const FMatrix Outline = FMatrix::Translation(FVector(-Center.x, -Center.y, -Center.z))
		* FMatrix::Scale(OutlineScale)
		* FMatrix::Translation(Center)
		* RI.WorldTransformMatrix;

	// 맵 캐싱 제거 및 ComPtr의 원시 포인터 전달
	mRenderer->RenderHighlight(
		RI.VertexBuffer->VertexBuffer.Get(),
		RI.VertexBuffer->NumVertices,
		mViewUnifiedProjectionMatrix,
		Outline,
		RI
	);
}


void FGraphicsManager::StartProjectionTransition(bool orthographic)
{
	mProjectionStartRatio = mProjectionRatio;
	mProjectionTargetRatio = orthographic ? 0.0f : 1.0f;
	mProjectionElapsed = 0.0f;

	mbProjectionTransitioning =
		mProjectionStartRatio != mProjectionTargetRatio;
}

bool FGraphicsManager::IsOrthographicTarget() const
{
	return mProjectionTargetRatio == 0.0f;
}

void FGraphicsManager::UpdateProjectionTransition(float deltaTime)
{
	if (!mbProjectionTransitioning)
	{
		return;
	}

	mProjectionElapsed += deltaTime;

	const float u = FMath::Clamp(
		mProjectionElapsed / mProjectionDuration, 0.0f, 1.0f);

	// Smoothstep interpolation for a smoother transition
	const float blend = u * u * (3.0f - 2.0f * u);

	mProjectionRatio = mProjectionStartRatio + (mProjectionTargetRatio - mProjectionStartRatio) * blend;

	if (u >= 1.0f)
	{
		mProjectionRatio = mProjectionTargetRatio;
		mbProjectionTransitioning = false;
	}
}
