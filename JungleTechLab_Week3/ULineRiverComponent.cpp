#include "ULineRiverComponent.h"
#include "GraphicsManager.h"
#include "SceneComponent.h"
#include "MathUtility.h"
#include "Console.h"
#include "JsonUtil.h"

void ULineRiverComponent::SerializeClass(json::JSON& outJson) const
{
	UPrimitiveComponent::SerializeClass(outJson);
	auto& Properties = outJson["Properties"];
	Properties["TotalLineNum"] = TotalLineNum;
	Properties["RiverWidth"] = RiverWidth;
	Properties["RiverLength"] = RiverLength;
	Properties["RiverDepth"] = RiverDepth;
	Properties["MinLineVelocity"] = MinLineVelocity;
	Properties["MaxLineVelocity"] = MaxLineVelocity;
	Properties["MinLineLength"] = MinLineLength;
	Properties["MaxLineLength"] = MaxLineLength;
	Properties["LineVertexCount"] = LineVertexCount;
	Properties["WaveSpeedWeight"] = WaveSpeedWeight;
	Properties["WavePhaseWeight"] = WavePhaseWeight;
}

void ULineRiverComponent::DeserializeClass(const json::JSON& inJson)
{
	UPrimitiveComponent::DeserializeClass(inJson);
	const auto& Properties = inJson.at("Properties");

	if (Properties.hasKey("TotalLineNum"))
		TotalLineNum = IntegerFromJson(Properties.at("TotalLineNum"), 0, 100000);
	if (Properties.hasKey("RiverWidth")) RiverWidth = NumberFromJson(Properties.at("RiverWidth"));
	if (Properties.hasKey("RiverLength")) RiverLength = NumberFromJson(Properties.at("RiverLength"));
	if (Properties.hasKey("RiverDepth")) RiverDepth = NumberFromJson(Properties.at("RiverDepth"));
	if (Properties.hasKey("MinLineVelocity")) MinLineVelocity = NumberFromJson(Properties.at("MinLineVelocity"));
	if (Properties.hasKey("MaxLineVelocity")) MaxLineVelocity = NumberFromJson(Properties.at("MaxLineVelocity"));
	if (Properties.hasKey("MinLineLength")) MinLineLength = NumberFromJson(Properties.at("MinLineLength"));
	if (Properties.hasKey("MaxLineLength")) MaxLineLength = NumberFromJson(Properties.at("MaxLineLength"));
	if (Properties.hasKey("LineVertexCount"))
		LineVertexCount = IntegerFromJson(Properties.at("LineVertexCount"), 1, 10000);
	if (Properties.hasKey("WaveSpeedWeight")) WaveSpeedWeight = NumberFromJson(Properties.at("WaveSpeedWeight"));
	if (Properties.hasKey("WavePhaseWeight")) WavePhaseWeight = NumberFromJson(Properties.at("WavePhaseWeight"));

	if (RiverWidth < 0.f || RiverLength <= 0.f || RiverDepth < 0.f ||
		MinLineVelocity < 0.f || MaxLineVelocity < MinLineVelocity ||
		MinLineLength <= 0.f || MaxLineLength < MinLineLength ||
		WaveSpeedWeight < 0.f || WavePhaseWeight < 0.f)
	{
		throw std::runtime_error("Invalid line river settings");
	}

	LineTime = 0.f;
	LocalLoc = GetForwardVector() * (RiverLength / 2.f) + GetRightVector() * (RiverWidth / 2.f);
	InitializeLineRiver();
	UpdateBounds();
}

void ULineRiverComponent::Initialize()
{
	TotalLineNum = 500;
	RiverWidth = 5.f;
	RiverLength = 50.f;
	RiverDepth = 0.01f;
	MinLineVelocity = 1.f;
	MaxLineVelocity = 2.f;
	MinLineLength = 0.5f;
	MaxLineLength = 2.f;

	LineVertexCount = 10;
	LineTime = 0.f;
	WaveSpeedWeight = 5.0f;
	WavePhaseWeight = 1.5f;

	LocalLoc = GetForwardVector() * (RiverLength / 2.f) + GetRightVector() * (RiverWidth / 2.f);
	InitializeLineRiver();

	SetRelativeScale3D(FVector(0.f, RiverWidth, RiverLength));
	PrimitiveColor = FVector4(1.f, 1.f, 1.f, 0.f);

	//SetRelativeLocation(FVector(RiverLength / 2.f, RiverWidth / 2.f, 0.f));
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
	Lines.Reset(TotalLineNum);
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
		//line.StartPoint = GetRelativeLocation() - LocalLoc + FVector(line.LineLength, line.LineWidth, 0.f);
		line.StartPoint = GetRelativeLocation() + FVector(line.LineLength, line.LineWidth, 0.f);
		line.LineDepth = DistDepth(RandomEngine);
		line.Movement = FVector::dot(line.StartPoint, GetForwardVector());
		line.StartPoint -= LocalLoc;
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
	FGraphicsManager::Get().DrawLine(GetRelativeLocation() - LocalLoc, GetRelativeLocation() - LocalLoc + GetForwardVector() * RiverLength, FVector4(1.f, 1.f, 1.f, 1.f));
	FGraphicsManager::Get().DrawLine(GetRelativeLocation() - LocalLoc + GetRightVector() * RiverWidth, GetRelativeLocation() - LocalLoc + GetRightVector() * RiverWidth + GetForwardVector() * RiverLength, FVector4(1.f, 1.f, 1.f, 1.f));

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
			line.StartPoint = GetRelativeLocation() - LocalLoc + FVector(0.f, line.LineWidth, line.LineDepth);
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
			float AddNoise = FMath::Sin(FMath::DegreesToRadians(Angle) * WavePhaseWeight + LineTime * WaveSpeedWeight);
			AddNoise *= 0.1f;
			End = Straight + GetForwardVector() * VertexSpacing + GetRightVector() * AddNoise;
			Straight += GetForwardVector() * VertexSpacing;

			//모든 라인을 Draw Line에 추가
			FGraphicsManager::Get().DrawLine(Start, End, FVector4(0.f, 0.f, 1.f - line.LineDepth, 1.f));
		}

		//line.StartPoint += GetForwardVector() * line.LineVelocity * dt;
		line.StartPoint = GetRelativeLocation() - LocalLoc + GetRightVector() * line.LineWidth + GetUpVector() * line.LineDepth + GetForwardVector() * line.Movement;
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
