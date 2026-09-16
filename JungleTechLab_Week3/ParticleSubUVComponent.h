#pragma once
#include "StaticMeshComponent.h"




// SaticMsehComponent를 사용하는 애들중에 SubUV기능을 사용하는 친구들
class UParticleSubUVComponent : public UStaticMeshComponent
{
	REFLECT_CLASS(UParticleSubUVComponent, UStaticMeshComponent)
public:
	UParticleSubUVComponent() = default;
	virtual ~UParticleSubUVComponent() {  }


	void Initialize();

	virtual void Update(TArray<FRenderInfo>* outRenderInfos, float DeltaTime) override;
	virtual void AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const override;
	void SetCols(int32 cols) { Cols = cols; }
	int32 GetCols() { return Cols; }
	void SetRows(int32 rows) { Rows = rows; }
	int32 GetRows() { return Rows; }

	void SerializeClass(json::JSON&) const override;
	void DeserializeClass(const json::JSON&) override;
	void SetScaleAndOffset(FVector4 sclaeOffset);
	void  SetPlayRate(float rate) { PlayRate = rate; }
	float GetPlayRate() const { return PlayRate; }
	void  SetLoop(bool loop) { bLoop = loop; }
	bool  GetLoop() const { return bLoop; }
	//AActor* SpawnSubUVActor(FTransform Transform, const FResourceManager& RM);
private:
	int32 Cols=4; 
	int32 Rows=4;
	
	float Elapsed=0.0f; //경과
	float PlayRate=10.0f; //초당 몇칸넘길까
	bool bLoop = true; 
	
	float scaleX=1;
	float scaleY=1;
	float offsetX=0;
	float offsetY=0;
};
