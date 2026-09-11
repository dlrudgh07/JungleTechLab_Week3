#include "QuadComponent.h"

UQuadComponent::UQuadComponent()
{
}

void UQuadComponent::Initialize()
{
	Initialize(FVector(0.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f));
}

void UQuadComponent::Initialize(FVector location, FRotator rotation, FVector scale3D)
{
	UPrimitiveComponent::Initialize(EPrimitive::EP_Quad, location, rotation, scale3D);
}

UQuadComponent::~UQuadComponent()
{
}
