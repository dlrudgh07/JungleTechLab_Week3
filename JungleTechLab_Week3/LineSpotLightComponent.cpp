#include "LineSpotLightComponent.h"
#include "ResourceManager.h"
#include "GraphicsManager.h"
#include "Actor.h"
#include <cmath>
#include "Vector.h"
#include "GraphicsManager.h"
#include "Transform.h"
#include "SceneSerialization.h"
void ULineSpotLightComponent::Initialize()
{
	HitMesh = FResourceManager::Get().GetStaticMesh("Sphere");   // 
	UpdateBounds();
	SetRelativeScale3D(FVector(0.3f));
		
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


void ULineSpotLightComponent::Update(TArray<FRenderInfo>* Out, float Dt)
{
	UPrimitiveComponent::Update(Out, Dt);   // FIX: 부모 호출
	UpdateBounds();                          // Transform 변경 감지가 없으니 매 프레임
	AddRenderInfos(Out);
	DrawCone();
}

void ULineSpotLightComponent::DrawCone() const
{
	
	FTransform OriginTransform = GetTransformMatrix();
	FMatrix R = OriginTransform.Rotation.ToMatrix() * FMatrix::Translation(OriginTransform.Location);
	//단위 : radian
	FVector T;
	T.y = tan(OuterAngle)*length;
	for (int t = 0;t < Segments;++t)
	{
		T.x = length * std::cos((float)t*PI*2/ Segments);
		T.z = length * std::sin((float)t*PI*2/ Segments);

		
	FGraphicsManager::Get().DrawLine(OriginTransform.Location, R.TransformPosition(FVector(T.x,T.y,T.z)), LineColor);
	}
	
}

void ULineSpotLightComponent::SerializeClass(json::JSON& OutJson) const
{
	UPrimitiveComponent::SerializeClass(OutJson);
	auto& P = OutJson["Properties"];
	P["OuterAngle"] = OuterAngle;
	P["Length"] = length;
	P["Segments"] = Segments;
	P["LineColor"] = FVector4ToJson(LineColor);
	P["Point"] = FVectorToJson(Point);
	P["HitMeshGUID"] = ObjectReference(HitMesh);
}

void ULineSpotLightComponent::DeserializeClass(const json::JSON& InJson)
{
	UPrimitiveComponent::DeserializeClass(InJson);
	const auto& P = InJson.at("Properties");
	if (P.hasKey("OuterAngle")) OuterAngle = NumberFromJson(P.at("OuterAngle"));
	if (P.hasKey("Length")) length = NumberFromJson(P.at("Length"));
	else if (P.hasKey("Range")) length = NumberFromJson(P.at("Range"));
	if (P.hasKey("Segments")) Segments = IntegerFromJson(P.at("Segments"), 1, 100000);
	if (length < 0) throw std::runtime_error("Invalid spotlight length");
	if (P.hasKey("LineColor")) LineColor = FVector4FromJson(P.at("LineColor"));
	if (P.hasKey("Point")) Point = FVectorFromJson(P.at("Point"));
	if (P.hasKey("HitMeshGUID")) HitMesh = ResolveReference<UStaticMesh>(P.at("HitMeshGUID"));
	UpdateBounds(); // Initialize would overwrite the restored scale.
}
