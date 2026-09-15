#include "UTextComponent.h"
#include "ResourceManager.h"
#include "FFontAsset.h"
#include "Texture.h"
#include "Vector.h"
#include "Renderer.h"
#include "GraphicsManager.h"
#include "StaticMesh.h"
#include "Actor.h"

UTextComponent::UTextComponent()
{
}

UTextComponent::~UTextComponent()
{
	Release();
}

const std::wstring& UTextComponent::GetText() const
{
	return Text;
}

void UTextComponent::SetText(const std::wstring& text)
{
	if (Text != text)
	{
		bDirty = true;
		Text = text;
	}
}

FFontAsset* UTextComponent::GetFontAsset() const
{
	return FontAsset;
}

void UTextComponent::SetFontAsset(FFontAsset* Font)
{
	FontAsset = Font;
}

void UTextComponent::BuildTextQuads()
{
	if (FontAsset == nullptr) return;
	//if (Text == L"")
	//{
	//	//기존 글자 버퍼 해제
	//	for (auto& Elem : PageBuffers)
	//	{
	//		Elem.second->VertexBuffer->Release();
	//		delete Elem.second;
	//	}

	//	PageBuffers.Reset();
	//	return;
	//}

	//page 별로 Vertices를 저장합니다.
	//key : page 인덱스
	//value : Vertices
	TMap<int32, TArray<FVertexSimple>> VerticesByPage;

	float penX = 0.f;
	float penY = 0.f;
	//Kerning 생기면 작업.
	//uint32 PrevChar = 0;

	//한글자씩 버퍼 채우기
	for (wchar_t ch : Text)
	{
		//한글자의 정보 가져오기
		const FCharacterInfo* Info = FontAsset->GetCharInfo((int32)ch);
		
		if (Info == nullptr)
		{
			//UE 로그가 한글도 지원하도록 나중에 작업해보자.
			//UE_LOG_F("{}에서 {}가 없습니다.", Text, ch);
			continue;
		}

		//penX에 Kerning의 값만큼 더해줘야한다.
		//penX += FontAsset->GetKerning(...);

		//글자의 텍스처 상 uv
		TArray<FVector2> uv = FontAsset->GetUV(*Info);

		assert(uv.Num() == 2);
		if (uv.Num() != 2) return;

		//버텍스 위치
		//시작지점인 penX,Y에서 Offset만큼 이동 후 Width, Height만큼 글자가 차지한다.
		float left = penX + Info->XOffset * FontScale;
		float top = penY + Info->YOffset* FontScale;
		float right = left + Info->Width* FontScale;
		float bottom = top + Info->Height* FontScale;

		//페이지에 해당하는 버텍스 채우기
		TArray<FVertexSimple>& Vertices = VerticesByPage[Info->Page];
		Vertices.Add({left, top, 0, 1,1,1,1, uv[0].x, uv[0].y});		//top left
		Vertices.Add({right, top, 0, 1,1,1,1, uv[1].x, uv[0].y});		//top right
		Vertices.Add({right, bottom, 0, 1,1,1,1, uv[1].x, uv[1].y});	//bottom right
		Vertices.Add({left, top, 0, 1,1,1,1, uv[0].x, uv[0].y});		//top left
		Vertices.Add({right, bottom, 0, 1,1,1,1, uv[1].x, uv[1].y});	//bottom right
		Vertices.Add({left, bottom, 0, 1,1,1,1, uv[0].x, uv[1].y});		//bottom left 


		//지점 이동
		penX += Info->XAdvance * FontScale;
		//PrevChar = ch;
		PenY = PenY < bottom ? bottom : PenY;
	}

	//큰 쿼드 만들기 용 저장
	PenX = penX;
	//PenY = penY;

	//기존 글자 버퍼 해제
	for (auto& Elem : PageBuffers)
	{
		Elem.second->VertexBuffer->Release();
		delete Elem.second;
	}

	PageBuffers.Reset();

	// 2) 페이지별로 각자 Dynamic 버퍼에 채워 넣기
	for (auto& [page, verts] : VerticesByPage)
	{
		uint32 Count = verts.Num() * sizeof(FVertexSimple);
		//UE_LOG("sizeof verts : %d", Count);
		// 초기 생성
		if (PageBuffers.Find(page) == nullptr)
			PageBuffers[page] = FGraphicsManager::Get().CreateDynamicBuffer(verts.Data(), Count);
		FGraphicsManager::Get().UpdateDynamicBuffer(PageBuffers[page]->VertexBuffer.Get(), verts.Data(), verts.Num());
	}

}

void UTextComponent::Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime)
{
	UPrimitiveComponent::Update(OutRenderInfos, DeltaTime);

	AddRenderInfos(OutRenderInfos);
}

FBoxSphereBounds UTextComponent::CalculateBounds(const FMatrix& LocalToWorld) const
{
	return LocalBounds.TransformBy(LocalToWorld);
}

void UTextComponent::GetVertices(std::vector<FVertexSimple>& OutVertices) const
{
	OutVertices = CPUVertices;
}

void UTextComponent::GetIndices(std::vector<uint32>& OutIndices) const
{
	
}

void UTextComponent::AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const
{
	if (FontAsset == nullptr) return;

	if (bDirty)
	{
		// dirty일 때만 재생성
		const_cast<UTextComponent*>(this)->BuildTextQuads();
		const_cast<UTextComponent*>(this)->SetHighLightQuadRenderInfo(outRenderInfos);
		const_cast<UTextComponent*>(this)->bDirty = false;
	}
	else
	{
		const_cast<UTextComponent*>(this)->HighLightInfo.WorldTransformMatrix = GetTransformMatrix().MakeMatrix();

		outRenderInfos->Add(const_cast<UTextComponent*>(this)->HighLightInfo);
	}

	//하이라이트 용 쿼드를 가장 첫번째 렌더 인포로 넣어준다.

	//Page에 따라 Render Info를 넣어준다.
	for (auto& [page, buffer] : PageBuffers)
	{
		if (!buffer || buffer->NumVertices == 0) continue;

		FRenderInfo Info;
		Info.VertexBuffer = buffer;
		Info.BaseTexture = FResourceManager::Get().GetTexture(FontAsset->GetPageName(page));
		Info.BlendMode = EBlendMode::Translucent;
		Info.WorldTransformMatrix = GetTransformMatrix().MakeMatrix();
		Info.LocalBoundsCenter = FVector(0.0f, 0.0f, 0.0f);
		Info.LocalBoundsHalfExtent = FVector(0.5f, 0.5f, 0.5f);
		Info.ObejctID = { Owner->ObjectID.GUID, Owner->ObjectID.InternalIndex };
		Info.Color = Color;
		//Info.Color = FVector4(1.f, 1.f, 1.f, 1.f); 
		outRenderInfos->Add(Info);
	}
}

void UTextComponent::Release()
{
	//PageBuffers
	for (auto& Pair : PageBuffers)
	{
		Pair.second->VertexBuffer->Release();
	}
	PageBuffers.Reset();
}

void UTextComponent::SetHighLightQuadRenderInfo(TArray<FRenderInfo>* outRenderInfos)
{
	if (bDirty)
	{
		CPUVertices.clear();

		//페이지에 해당하는 버텍스 채우기
		TArray<FVertexSimple>Vertices;
		//문자열 quad와 같은 위치면 겹쳐버려서 0.001만큼 뒤로 밉니다.
		FVertexSimple TopLeft = { 0.f, 0.f, -0.001f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f };
		FVertexSimple TopRight = { PenX, 0.f, -0.001f, 1.f, 1.f, 1.f, 1.f, 1.f, 0.f };
		FVertexSimple BottomLeft = { 0.f, PenY, -0.001f, 1.f, 1.f, 1.f, 1.f, 0.f, 1.f };
		FVertexSimple BottomRight = { PenX, PenY, -0.001f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }; 
		Vertices.Add(TopLeft);		//top left
		Vertices.Add(TopRight);		//top right
		Vertices.Add(BottomLeft);	//bottom left
		Vertices.Add(BottomLeft);	//bottom left
		Vertices.Add(TopRight);		//top right
		Vertices.Add(BottomRight);	//bottom right

		CPUVertices.push_back(TopLeft);		//top left
		CPUVertices.push_back(TopRight);		//top right
		CPUVertices.push_back(BottomLeft);	//bottom right
		CPUVertices.push_back(BottomLeft);	//bottom right
		CPUVertices.push_back(TopRight);		//top right
		CPUVertices.push_back(BottomRight);	//bottom left

		//하이라이트 버퍼 생성
		uint32 Count = Vertices.Num() * sizeof(FVertexSimple);
		HighLightBuffer = FGraphicsManager::Get().CreateDynamicBuffer(Vertices.Data(), Count);
		FGraphicsManager::Get().UpdateDynamicBuffer(HighLightBuffer->VertexBuffer.Get(), Vertices.Data(), Vertices.Num());

		//Bounding Box
		CalculateLocalBounds();
		UpdateBounds();

		HighLightInfo.VertexBuffer = HighLightBuffer;
		HighLightInfo.BaseTexture = FResourceManager::Get().GetTexture(FontAsset->GetPageName(0));
		HighLightInfo.BlendMode = EBlendMode::Translucent; 
		HighLightInfo.LocalBoundsCenter = LocalBounds.Center;
		HighLightInfo.LocalBoundsHalfExtent = LocalBounds.BoxHalfExtent;
		HighLightInfo.ObejctID = { Owner->ObjectID.GUID, Owner->ObjectID.InternalIndex };
		HighLightInfo.Color = FVector4(1.f, 1.f, 1.f, 0.f);
	}

	HighLightInfo.WorldTransformMatrix = GetTransformMatrix().MakeMatrix();

	outRenderInfos->Add(HighLightInfo);
}

void UTextComponent::CalculateLocalBounds()
{
	if (CPUVertices.empty())
	{
		Bounds = FBoxSphereBounds(); // 정점이 없으면 Invalid 기본값
		return;
	}

	FVector MinBound(FLT_MAX, FLT_MAX, FLT_MAX);
	FVector MaxBound(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	//Min, Max 계산
	for (const FVertexSimple& Vertex : CPUVertices)
	{
		// X축 최소/최대
		if (Vertex.x < MinBound.x) MinBound.x = Vertex.x;
		if (Vertex.x > MaxBound.x) MaxBound.x = Vertex.x;

		// Y축 최소/최대
		if (Vertex.y < MinBound.y) MinBound.y = Vertex.y;
		if (Vertex.y > MaxBound.y) MaxBound.y = Vertex.y;

		// Z축 최소/최대
		if (Vertex.z < MinBound.z) MinBound.z = Vertex.z;
		if (Vertex.z > MaxBound.z) MaxBound.z = Vertex.z;
	}

	// 구한 Min, Max를 통해 LocalBounds 생성 (FBoxSphereBounds 생성자가 Center와 Extent를 자동 계산)
	//Bounds = FBoxSphereBounds(MinBound, MaxBound); 
	LocalBounds = FBoxSphereBounds(MinBound, MaxBound);
}

