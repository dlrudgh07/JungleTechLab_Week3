#pragma once
#include "StaticMeshComponent.h"
#include "RenderInfo.h"
#include "TArray.h"
#include "Vector.h"

#include <random>
#include <iostream>

struct FLineRiver
{
	FVector StartPoint = FVector(0.f, 0.f, 0.f);
	float LineLenth = 0.f;
	float LineVelocity = 0.f;
	float Movement = 0.f;
	float LineDepth = 0.f;
	float LineWidth = 0.f;
	float LineLength = 0.f;
};

class ULineRiverComponent :
    public UPrimitiveComponent
{
public:
	void SerializeClass(json::JSON& outJson) const override;
	void DeserializeClass(const json::JSON& inJson) override;

	ULineRiverComponent() = default;
	virtual ~ULineRiverComponent() = default;

	// factory 에서 필요함
	void Initialize();

	virtual void Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime) override;	

	//void AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const override;

	void GetVertices(std::vector<FVertexSimple>& OutVertices)const override;
	void GetIndices(std::vector<uint32>& OutIndices)const override;

	//첫 라인강 초기화
	void InitializeLineRiver();

	//시간에 맞춰 라인의 상태를 업데이트합니다.
	void UpdateLine(float dt);

	float GetRiverWidth()const;
	float GetRiverLength()const;

	REFLECT_CLASS(ULineRiverComponent, UPrimitiveComponent)

private:
	//라인들을 저장
	TArray<FLineRiver> Lines;
	//총 라인의 개수
	int32 TotalLineNum;
	//강의 폭
	float RiverWidth;
	//강의 길이
	float RiverLength;
	//강의 깊이
	float RiverDepth;
	float MinLineVelocity;
	float MaxLineVelocity;
	float MinLineLength;
	float MaxLineLength;

	//라인 하나에 들어가는 버텍스 수
	int32 LineVertexCount;

	std::mt19937 RandomEngine{ std::random_device{}() };

	float LineTime;

	//sin 파형 웨이브 속도
	float WaveSpeedWeight;

};

