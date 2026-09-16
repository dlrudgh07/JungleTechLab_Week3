#pragma once

#include "SceneComponent.h"
#include "Vector.h"

// 파티클의 현재 상태를 나타내는 열거형
enum class EParticleState
{
	Inactive,   // 대기 상태 (풀에서 사용 가능)
	Falling,    // 비가 내리는 중
	Splashing   // 바닥에 부딪혀 튀는 중
};

// 낙하하는 빗방울 구조체
struct FRainDrop
{
	FVector3 Position;
	FVector3 Velocity;
	EParticleState State = EParticleState::Inactive;
};

// 튀어 오르는 물보라 구조체
struct FSplashDrop
{
	FVector3 Position;
	FVector3 Velocity;
	float LifeTime = 0.0f;
	float MaxLifeTime = 0.0f;
	EParticleState State = EParticleState::Inactive;
};

class UParticleRainComponent : public USceneComponent
{
public:

	// factory 에서 필요함
	void Initialize()
	{
		RainPool.resize(MaxRainDrops);
		SplashPool.resize(MaxSplashDrops);
	};
	virtual ~UParticleRainComponent() {};

	virtual void SerializeClass(json::JSON& OutJson) const override;
	virtual void DeserializeClass(const json::JSON& InJson) override;

	// 렌더 패스에서 라인을 그리기 위해 호출될 함수
	void RenderRain();

	virtual void Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime) override;

	REFLECT_CLASS(UParticleRainComponent, USceneComponent)

public:
	// [파티클 시스템 설정값]
	int32 MaxRainDrops = 3000;      // 최대 빗방울 수
	int32 MaxSplashDrops = 9000;    // 최대 물보라 수 (보통 비 1방울당 3개)

	float SpawnRadius = 6.0f;    // 액터 중심 비 생성 반경
	float SpawnHeight = 80.0f;     // 액터 중심 비 생성 높이
	float FloorZ = 0.0f;            // 바닥 충돌 높이 (나중엔 레이캐스트로 발전 가능)

	float RainDropSpeed = 130.0f;  // 비 낙하 속도
	float Gravity = 20.0f;        // 물보라에 적용될 중력

	float RainSpawnRate = 0.01f;    // 비 생성 주기 (초)

private:
	// 오브젝트 풀
	std::vector<FRainDrop> RainPool;
	std::vector<FSplashDrop> SplashPool;

	// 스폰 타이머
	float RainSpawnTimer = 0.0f;

	// 내부 헬퍼 함수
	void SpawnRainDrop();
	void SpawnSplash(const FVector3& ImpactPosition);

	// 랜덤 유틸리티 (FMath::FRandRange 등으로 대체 가능)
	float GetRandomFloat(float Min, float Max) const;
};
