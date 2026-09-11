#include "UTextComponent.h"

UTextComponent::UTextComponent()
{
}

void UTextComponent::Initialize()
{
	Initialize(FVector(0.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f));
}

void UTextComponent::Initialize(FVector location, FRotator rotation, FVector scale3D)
{
	UPrimitiveComponent::Initialize(EPrimitive::EP_Text, location, rotation, scale3D);
}

UTextComponent::~UTextComponent()
{
}
