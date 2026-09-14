#include "ParticleSubUVComponent.h"

void UParticleSubUVComponent::Initialize()
{

}


void UParticleSubUVComponent::Update(TArray<FRenderInfo>* outRenderInfos, float DeltaTime)
{
	Elapsed += DeltaTime;

	UStaticMeshComponent::Update(outRenderInfos, DeltaTime);
}

void UParticleSubUVComponent::AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const
{
	const int32 Before = outRenderInfos->Num(); 
	UStaticMeshComponent::AddRenderInfos(outRenderInfos);

	// 부모가 조기 return 했으면(메시 없음) 건드릴 게 없다
	if (outRenderInfos->Num() == Before)
		return;

	int32 Total = Cols * Rows;
	int32 UVIndex = static_cast<int32> (Elapsed * PlayRate);
	if (bLoop) { UVIndex = UVIndex % Total; } // 범위초과시 다시 0으로
	else
	{
		if (UVIndex >= Total) UVIndex = Total - 1;
	}

	const int32 Col = UVIndex % static_cast<int32>(Cols);
	const int32 Row = UVIndex / static_cast<int32>(Cols);


	FRenderInfo& Info = (*outRenderInfos)[outRenderInfos->Num() - 1];
	Info.UVTransform.x = 1.0f/Cols;
	Info.UVTransform.y = 1.0f/Rows;
	Info.UVTransform.z = Col / static_cast<float>(Cols);
	Info.UVTransform.w = Row / static_cast<float>(Rows);
}

void UParticleSubUVComponent::SetScaleAndOffset(FVector4 scaleoffset)
{
	scaleX = scaleoffset.x;
	scaleY = scaleoffset.y;
	offsetX = scaleoffset.z;
	offsetY = scaleoffset.w;
}
