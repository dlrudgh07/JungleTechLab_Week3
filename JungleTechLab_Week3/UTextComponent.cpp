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
	if (FontAsset == nullptr || Text == L"") return;

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
		if (Info == nullptr) continue;

		//penX에 Kerning의 값만큼 더해줘야한다.
		//penX += FontAsset->GetKerning(...);

		//글자의 텍스처 상 uv
		TArray<FVector2> uv = FontAsset->GetUV(*Info);

		//버텍스 위치
		//시작지점인 penX,Y에서 Offset만큼 이동 후 Width, Height만큼 글자가 차지한다.
		float left = penX + Info->XOffset;
		float top = penY + Info->YOffset;
		float right = left + Info->Width;
		float bottom = top + Info->Height;

		//페이지에 해당하는 버텍스 채우기
		TArray<FVertexSimple>& Vertices = VerticesByPage[Info->Page];

		Vertices.Add({left, top, 0, 1,1,1,1, uv[0].x, uv[0].y});		//top left
		Vertices.Add({right, top, 0, 1,1,1,1, uv[1].x, uv[0].y});		//top right
		Vertices.Add({right, bottom, 0, 1,1,1,1, uv[1].x, uv[1].y});	//bottom right
		Vertices.Add({left, top, 0, 1,1,1,1, uv[0].x, uv[0].y});		//top left
		Vertices.Add({right, bottom, 0, 1,1,1,1, uv[1].x, uv[1].y});	//bottom right
		Vertices.Add({left, bottom, 0, 1,1,1,1, uv[0].x, uv[1].y});		//bottom left

		//지점 이동
		penX += Info->XAdvance;
		//PrevChar = ch;
	}

	// 2) 페이지별로 각자 Dynamic 버퍼에 채워 넣기
	for (auto& [page, verts] : VerticesByPage)
	{
		// 초기 생성
		if (PageBuffers.Find(page) == nullptr)
			PageBuffers[page] = FGraphicsManager::Get().CreateDynamicBuffer(verts.Data(), verts.Num());
		FGraphicsManager::Get().UpdateDynamicBuffer(PageBuffers[page]->VertexBuffer.Get(), verts.Data(), verts.Num());
	}

	bDirty = false;
}

void UTextComponent::Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime)
{
	UPrimitiveComponent::Update(OutRenderInfos, DeltaTime);

	AddRenderInfos(OutRenderInfos);
}

void UTextComponent::AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const
{
	if (bDirty)
	{
		// dirty일 때만 재생성
		const_cast<UTextComponent*>(this)->BuildTextQuads();
	}

	//Page에 따라 Render Info를 넣어준다.
	for (auto& [page, buffer] : PageBuffers)
	{
		if (!buffer || buffer->NumVertices == 0) continue;

		FRenderInfo Info;
		Info.VertexBuffer = buffer;
		Info.BaseTexture = FResourceManager::Get().GetTexture(FontAsset->GetPageName(page));
		Info.WorldTransformMatrix = GetTransformMatrix().MakeMatrix();
		Info.BoundsCenter = FVector(0.0f, 0.0f, 0.0f);
		Info.BoundsHalfExtent = FVector(0.5f, 0.5f, 0.5f);
		Info.ObejctID = { Owner->ObjectID.GUID, Owner->ObjectID.InternalIndex };
		Info.Color = Color;
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
