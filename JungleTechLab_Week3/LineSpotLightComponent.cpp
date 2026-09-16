#include "LineSpotLightComponent.h"
#include "ResourceManager.h"
#include "GraphicsManager.h"
#include "Actor.h"
#include <cmath>
#include "Vector.h"
#include "GraphicsManager.h"
void ULineSpotLightComponent::Initialize()
{
	HitMesh = FResourceManager::Get().GetStaticMesh("Sphere");   // 
	UpdateBounds();
}

void ULineSpotLightComponent::AddRenderInfos(TArray<FRenderInfo>* Out) const
{
	if (HitMesh == nullptr || HitMesh->VertexBuffer == nullptr) return;   // FIX: 가드

	FRenderInfo Info;
	Info.VertexBuffer = HitMesh->VertexBuffer;
	Info.CollisionVertices = HitMesh->CPUVertices.data();
	Info.CollisionVertexCount = static_cast<uint32>(HitMesh->CPUVertices.size());
	Info.LocalBoundsCenter = HitMesh->LocalBounds.Center;
	Info.LocalBoundsHalfExtent = HitMesh->LocalBounds.BoxHalfExtent;
	Info.BlendMode = EBlendMode::Translucent;
	Info.Color = FVector4(0.f, 0.f, 0.f, 1.0f);           // FIX: 알파 0이면 안 보임
	Info.WorldTransformMatrix = GetTransformMatrix().MakeMatrix();
	Info.ObejctID = { Owner->ObjectID.GUID, Owner->ObjectID.InternalIndex };

	Out->Add(Info);
}

FBoxSphereBounds ULineSpotLightComponent::CalculateBounds(const FMatrix& L2W) const
{
	if (HitMesh == nullptr) return FBoxSphereBounds();
	return HitMesh->LocalBounds.TransformBy(L2W);
}

FBoxSphereBounds ULineSpotLightComponent::GetLocalBounds() const
{
	return HitMesh ? HitMesh->LocalBounds : FBoxSphereBounds{};
}

void ULineSpotLightComponent::GetVertices(std::vector<FVertexSimple>& OutVertices) const
{
}

void ULineSpotLightComponent::GetIndices(std::vector<uint32>& OutIndices) const
{
}

void ULineSpotLightComponent::Update(TArray<FRenderInfo>* Out, float Dt)
{
	UPrimitiveComponent::Update(Out, Dt);   // FIX: 부모 호출
	UpdateBounds();                          // Transform 변경 감지가 없으니 매 프레임
	AddRenderInfos(Out);
	DrawCone();
}

void ULineSpotLightComponent::DrawCone() const
{
	FVector T= GetTransformMatrix().MakeMatrix().TransformPosition(0);
	//단위 : radian
	T.y = tan(OuterAngle)*length;
	for (int t = 0;t < 360;t += circleDensity)
	{
		T.x = length * std::cos((float)t*PI/180);
		T.z = length * std::sin((float)t*PI/180);
	FGraphicsManager::Get().DrawLine(Point, T,LineColor);
	}
	
}

void ULineSpotLightComponent::SerializeClass(json::JSON& OutJson) const
{
//	UPrimitiveComponent::SerializeClass(OutJson);   // FIX: 부모 호출 (Transform 저장)
//	OutJson["Properties"]["OuterAngle"] = OuterAngle;
//	OutJson["Properties"]["Range"] = Range;
//	OutJson["Properties"]["Segments"] = Segments;
}

void ULineSpotLightComponent::DeserializeClass(const json::JSON& InJson)
{
	UPrimitiveComponent::DeserializeClass(InJson);
//	const auto& P = InJson.at("Properties");
//	if (P.hasKey("OuterAngle")) OuterAngle = static_cast<float>(P.at("OuterAngle").ToFloat());
//	if (P.hasKey("Range"))      Range = static_cast<float>(P.at("Range").ToFloat());
//	if (P.hasKey("Segments"))   Segments = static_cast<int32>(P.at("Segments").ToInt());
//	Initialize();   // FIX: LoadObject는 Initialize를 안 부른다
}
