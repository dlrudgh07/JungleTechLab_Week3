
#include "PrimitiveComponent.h"

#include <format>

#include "RenderInfo.h"
#include "enum.h"
#include "JsonUtil.h"
#include "Console.h"
#include "Actor.h"

#include "FTextureManager.h"

UPrimitiveComponent::UPrimitiveComponent()
{
}

/*
void UPrimitiveComponent::Initialize(GraphicsManager* graphicsManager, EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D)
{
	USceneComponent::Initialize(location, rotation, scale3D);

	mGraphicsManager = graphicsManager;
	mePrimitive = ePrimitive;
}
*/

void UPrimitiveComponent::Initialize(EPrimitive ePrimitive)
{
	Initialize(ePrimitive, FVector(0.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f));

	if (ePrimitive != EPrimitive::EP_Quad)
		SubUVAnimator.SetIsPlaying(false);
}

void UPrimitiveComponent::Initialize(EPrimitive ePrimitive, FVector location, FRotator rotation, FVector scale3D)
{
	USceneComponent::Initialize(location, rotation, scale3D);

	mePrimitive = ePrimitive;
}

UPrimitiveComponent::~UPrimitiveComponent()
{
}

void UPrimitiveComponent::SerializeClass(json::JSON& outJson) const
{
	USceneComponent::SerializeClass(outJson);
	outJson["Properties"]["mePrimitiveType"] = EPrimitiveToJson(mePrimitive);
}

void UPrimitiveComponent::DeserializeClass(const json::JSON& inJson)
{
	USceneComponent::DeserializeClass(inJson);

	const json::JSON& propertiesJson = inJson.at("Properties");
	if (!propertiesJson.hasKey("mePrimitiveType") || propertiesJson.at("mePrimitiveType").JSONType() != json::JSON::Class::String)
	{
		throw std::runtime_error(std::format("{}: mePrimitiveType property requires a string", GetRuntimeClass()->Name));
	}

	mePrimitive = EPrimitiveFromJson(propertiesJson.at("mePrimitiveType"));
}

void UPrimitiveComponent::Update(TArray<FRenderInfo>* outRenderInfos, float DeltaTime)
{
	// Todo: Update coordinates here
	{
		//UE_LOG("Primitive selected");
	}

	SubUVAnimator.Update(DeltaTime);
	
	GetRenderInfos(outRenderInfos);
}


FVector4 GetUVScaleOffsetFontAtlas(char InputCharacter)
{
	FVector4 ResultScaleOffset{};

	/*
* todo
어차피 이 코드는 다른곳으로 가야함
기능 구현 위주
- cell width/height = 32
- row/col count = 16
	*/


	// 16 x 16 그리드
	int Cols = 16;
	int Rows = 16;

	// 1. 배율 (Scale) : 한 칸이 차지하는 UV 비율 (1/16 = 0.0625)
	float ScaleU = 1.0f / Cols;
	float ScaleV = 1.0f / Rows;
	ResultScaleOffset.x = ScaleU;
	ResultScaleOffset.y = ScaleV;

	// 2. 현재 글자의 행열 인덱스 계산
	// ASCII 코드를 16으로 나눈 몫 = 행(Row, Y)
	// ASCII 코드를 16으로 나눈 나머지 = 열(Col, X)
	int ColIndex = InputCharacter % Cols;
	int RowIndex = InputCharacter / Cols;

	// 3. 오프셋 (Offset) : UV 좌표계에서의 시작 위치
	ResultScaleOffset.z = ColIndex * ScaleU; // U Offset (가로 이동)
	ResultScaleOffset.w = RowIndex * ScaleV; // V Offset (세로 이동)

	return ResultScaleOffset;
}

void UPrimitiveComponent::GetRenderInfos(TArray<FRenderInfo>* outRenderInfos) const
{
	assert(outRenderInfos);

	// texture mapping
	// todo : 경로도 이렇게 하드코딩하지 않고, 제대로 설정해야함
	// todo : primitive에 직접적으로 texture를 매핑하는게 아니라, actor에 들어있는 특정 rendering 관련 component에서 이걸 결정하고, 그 값을 가져오는 방향으로 해야함
	// todo : 일단 cube일때만 하드코딩된 나무상자 넘기도록함
	// todo : quad도 동일함
	// todo : text도 동일함
	FTexture* MyCubeTexture = nullptr;
	if(mePrimitive == EPrimitive::EP_Cube)
		MyCubeTexture = FTextureManager::GetManager().LoadTexture("C:/Users/JUNGLE/Desktop/GameEngine/Week3/JungleTechLab_Week3/JungleTechLab_Week3/crate.png");
	else if (mePrimitive == EPrimitive::EP_Quad)
		MyCubeTexture = FTextureManager::GetManager().LoadTexture("C:/Users/JUNGLE/Desktop/GameEngine/Week3/JungleTechLab_Week3/JungleTechLab_Week3/fire_atlas.png");
	else if (mePrimitive == EPrimitive::EP_Text)
		MyCubeTexture = FTextureManager::GetManager().LoadTexture("C:/Users/JUNGLE/Desktop/GameEngine/Week3/JungleTechLab_Week3/JungleTechLab_Week3/english_atlas.png");


	// todo
	// 여기서 로직 처리하면 안됨
	// 따로 빼야함
	if(mePrimitive == EPrimitive::EP_Quad)
		outRenderInfos->Add({ mePrimitive, GetTransformMatrix().MakeMatrix(),{ mOwner->UUID, mOwner->InternalIndex }, FVector4(0, 0, 0, 0), MyCubeTexture
						, SubUVAnimator.GetUVScaleOffset() });
	else if (mePrimitive == EPrimitive::EP_Text)
	{
		// 일단 한글자 잘 그려지는지 확인
		outRenderInfos->Add({ mePrimitive, GetTransformMatrix().MakeMatrix(),{ mOwner->UUID, mOwner->InternalIndex }, FVector4(0, 0, 0, 0), MyCubeTexture
				, GetUVScaleOffsetFontAtlas('a')});
	}
	else
		outRenderInfos->Add({ mePrimitive, GetTransformMatrix().MakeMatrix(),{ mOwner->UUID, mOwner->InternalIndex }, FVector4(0, 0, 0, 0), MyCubeTexture
						, FVector4{ 1,1,0,0 } });
}

/*
void UPrimitiveComponent::Render(FStruct)
{
	// Todo: Fix renderer
	mGraphicsManager->Render(GetTransformMatrix(), mePrimitive);
}
*/


