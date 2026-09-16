#include "ULineRiverComponent.h"
#include "GraphicsManager.h"
#include "SceneComponent.h"
#include "MathUtility.h"
#include "Console.h"

void ULineRiverComponent::SerializeClass(json::JSON& outJson) const
{
}

void ULineRiverComponent::DeserializeClass(const json::JSON& inJson)
{
}

void ULineRiverComponent::Initialize()
{
	TotalLineNum = 300;
	RiverWidth = 5.f;
	RiverLength = 50.f;
	RiverDepth = 0.5f;
	MinLineVelocity = 1.f;
	MaxLineVelocity = 2.f;
	MinLineLength = 0.5f;
	MaxLineLength = 2.f;

	LineVertexCount = 10;
	LineTime = 0.f;
	WaveSpeedWeight = 3.0f;

	InitializeLineRiver();

	SetRelativeScale3D(FVector(0.f, RiverWidth, RiverLength));
	PrimitiveColor = FVector4(1.f, 1.f, 1.f, 0.f);


}

void ULineRiverComponent::Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime)
{
	//여기서 StaticMesh의 RenderInfo는 넘어갈것이다.
	UPrimitiveComponent::Update(OutRenderInfos, DeltaTime);

	UpdateLine(DeltaTime);
}

//void ULineRiverComponent::AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const
//{
//	//피킹 용 메시는 RenderInfo를 넣는다
//	UStaticMeshComponent::AddRenderInfos(outRenderInfos);
//}

void ULineRiverComponent::GetVertices(std::vector<FVertexSimple>& OutVertices) const
{
}

void ULineRiverComponent::GetIndices(std::vector<uint32>& OutIndices) const
{
}

void ULineRiverComponent::InitializeLineRiver()
{
	std::uniform_real_distribution<float> DistWidth(0.0f, RiverWidth);
	std::uniform_real_distribution<float> DistLength(0.0f, RiverLength);
	std::uniform_real_distribution<float> DistDepth(0.0f, RiverDepth);
	std::uniform_real_distribution<float> DistVelocity(MinLineVelocity, MaxLineVelocity);
	std::uniform_real_distribution<float> DistLineLength(MinLineLength, MaxLineLength);

	for(int i = 0; i < TotalLineNum; i++)
	{
		FLineRiver line;
		line.LineLenth = DistLineLength(RandomEngine);
		line.LineVelocity = DistVelocity(RandomEngine);
		line.LineWidth = DistWidth(RandomEngine);
		line.LineLength = DistLength(RandomEngine);
		line.StartPoint = GetRelativeLocation() + FVector(line.LineLength, line.LineWidth, 0.f);
		line.LineDepth = DistDepth(RandomEngine);
		line.Movement = FVector::dot(line.StartPoint, GetForwardVector());
		Lines.Add(line);
	}
}

void ULineRiverComponent::UpdateLine(float dt)
{
	LineTime += dt;
	if (LineTime > 100.f) LineTime = 0.f;

	std::uniform_real_distribution<float> DistWidth(0.0f, RiverWidth);
	std::uniform_real_distribution<float> DistDepth(0.0f, RiverDepth);

	//외각 라인 그려주기
	FGraphicsManager::Get().DrawLine(GetRelativeLocation(), GetRelativeLocation() + GetForwardVector() * RiverLength, FVector4(1.f, 1.f, 1.f, 1.f));
	FGraphicsManager::Get().DrawLine(GetRelativeLocation() + GetRightVector() * RiverWidth, GetRelativeLocation() + GetRightVector() * RiverWidth + GetForwardVector() * RiverLength, FVector4(1.f, 1.f, 1.f, 1.f));

	for (FLineRiver& line : Lines)
	{
		//시간에 맞춰 Movement 값 상승
		line.Movement += line.LineVelocity * dt;

		if (line.Movement > RiverLength)
		{
			//끝에 도착하면 시작점 초기화, movement 초기화
			line.Movement = 0.f;
			line.LineWidth = DistWidth(RandomEngine);
			line.LineDepth = DistDepth(RandomEngine);
			line.StartPoint = GetRelativeLocation() + FVector(0.f, line.LineWidth, line.LineDepth);
		}
				
		//버텍스 사이 간격 : 길이 / 버텍스 수
		float VertexSpacing = line.LineLenth / (float)LineVertexCount;
		float NoiseSpacing = 360.f / (float)LineVertexCount;

		FVector Start = line.StartPoint;
		FVector Straight = line.StartPoint;
		FVector End = line.StartPoint;

		//버텍스 수대로 그리기 추가
		for (int32 i = 0; i < LineVertexCount; i++)
		{
			Start = End;
			float Angle = NoiseSpacing * (i + 1);
			float AddNoise = FMath::Sin(FMath::DegreesToRadians(Angle) + LineTime * WaveSpeedWeight);
			AddNoise *= 0.1f;
			End = Straight + GetForwardVector() * VertexSpacing + GetRightVector() * AddNoise;
			Straight += GetForwardVector() * VertexSpacing;

			//모든 라인을 Draw Line에 추가
			//FGraphicsManager::Get().DrawLine(Start, End, FVector4((float) i / 10.f, (float)i / 15.f, (float)i / 20.f, 1.f));
			FGraphicsManager::Get().DrawLine(Start, End, FVector4(0.f, 0.f, 1.f - line.LineDepth, 1.f));
		}

		//line.StartPoint += GetForwardVector() * line.LineVelocity * dt;
		line.StartPoint = GetRelativeLocation() + GetRightVector() * line.LineWidth + GetUpVector() * line.LineDepth + GetForwardVector() * line.Movement;
	}
}

float ULineRiverComponent::GetRiverWidth() const
{
	return RiverWidth;
}

float ULineRiverComponent::GetRiverLength() const
{
	return RiverLength;
}
