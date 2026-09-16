#include "ParticleRainComponent.h"
#include "GraphicsManager.h" 
#include <cstdlib>           


float UParticleRainComponent::GetRandomFloat(float Min, float Max) const
{
	float RandomRatio = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	return Min + RandomRatio * (Max - Min);
}

void UParticleRainComponent::Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime) 
{
	USceneComponent::Update(OutRenderInfos, DeltaTime);

	// -------------------------------------------------------------
	// 1. 빗방울 스폰 로직 (타이머 기반)
	// -------------------------------------------------------------
	RainSpawnTimer += DeltaTime;
	while (RainSpawnTimer >= RainSpawnRate)
	{
		// 빗방울 양을 늘리고 싶다면 한 번에 여러 개 스폰
		for (int i = 0; i < 5; ++i)
		{
			SpawnRainDrop();
		}
		RainSpawnTimer -= RainSpawnRate;
	}

	// -------------------------------------------------------------
	// 2. 빗방울 물리 업데이트 (낙하 및 충돌)
	// -------------------------------------------------------------
	for (auto& Drop : RainPool)
	{
		if (Drop.State != EParticleState::Falling) continue;

		// 위치 갱신 (Z-Up 낙하)
		Drop.Position.z -= RainDropSpeed * DeltaTime;

		// 바닥 충돌 체크
		if (Drop.Position.z <= FloorZ)
		{
			Drop.Position.z = FloorZ;               // 바닥에 위치 고정
			Drop.State = EParticleState::Inactive;  // 빗방울 소멸 (풀 반환)

			SpawnSplash(Drop.Position);             // 스플래시 생성
		}
	}

	// -------------------------------------------------------------
	// 3. 물보라(Splash) 물리 업데이트
	// -------------------------------------------------------------
	for (auto& Splash : SplashPool)
	{
		if (Splash.State != EParticleState::Splashing) continue;

		// 중력 적용 (Z-Up 포물선)
		Splash.Velocity.z -= Gravity * DeltaTime;

		// 위치 갱신
		Splash.Position += Splash.Velocity * DeltaTime;

		// 수명 갱신
		Splash.LifeTime -= DeltaTime;
		if (Splash.LifeTime <= 0.0f)
		{
			Splash.State = EParticleState::Inactive; // 수명 다하면 소멸
		}
	}

	RenderRain();
}

void UParticleRainComponent::SerializeClass(json::JSON& OutJson) const
{
	USceneComponent::SerializeClass(OutJson);

	// 현재는 동적으로 변화를 주지 않기때문에 저장할 값도 없음
}

void UParticleRainComponent::DeserializeClass(const json::JSON& InJson)
{
	USceneComponent::DeserializeClass(InJson);

	// 로드할때는 initialize()를 호출하여 rain drop이 시작되도록 하자
	Initialize();
}

void UParticleRainComponent::SpawnRainDrop()
{
	// 컴포넌트의 현재 로컬 위치
	FVector3 ComponentLocation = GetRelativeLocation();

	// 풀에서 비어있는(Inactive) 빗방울 찾기
	for (auto& Drop : RainPool)
	{
		if (Drop.State == EParticleState::Inactive)
		{
			// 원형 범위 내 임의의 X, Y 좌표 계산 (Z-Up 환경이므로 평면은 X, Y)
			float RandAngle = GetRandomFloat(0.0f, 3.141592f * 2.0f);
			float RandRadius = GetRandomFloat(0.0f, SpawnRadius);

			float OffsetX = cosf(RandAngle) * RandRadius;
			float OffsetY = sinf(RandAngle) * RandRadius;

			// 로컬 공간상에 스폰 (컴포넌트 위치 기준 + Z축 높이)
			Drop.Position = FVector3(
				ComponentLocation.x + OffsetX,
				ComponentLocation.y + OffsetY,
				ComponentLocation.z + SpawnHeight
			);

			Drop.Velocity = FVector3(0.0f, 0.0f, -RainDropSpeed); // 아래(Z축 음수)로 낙하
			Drop.State = EParticleState::Falling;

			break; // 하나 스폰했으므로 종료
		}
	}
}

void UParticleRainComponent::SpawnSplash(const FVector3& ImpactPosition)
{
	int32 SplashCountToSpawn = 3; // 빗방울 하나당 생성할 물보라 개수
	int32 SpawnedCount = 0;

	for (auto& Splash : SplashPool)
	{
		if (Splash.State == EParticleState::Inactive)
		{
			Splash.Position = ImpactPosition;

			// [Z-Up 수정 및 스케일 축소]
			// X, Y는 바닥으로 퍼지는 방향, Z는 위로 튀어오르는 방향
			float VelocityX = GetRandomFloat(-10.0f, 10.0f);
			float VelocityY = GetRandomFloat(-10.0f, 10.0f);
			float VelocityZ = GetRandomFloat(5.0f, 10.0f); // Z축 튀어오름

			Splash.Velocity = FVector3(VelocityX, VelocityY, VelocityZ);

			Splash.MaxLifeTime = GetRandomFloat(0.1f, 0.25f); // 0.1~0.25초 생존
			Splash.LifeTime = Splash.MaxLifeTime;
			Splash.State = EParticleState::Splashing;

			SpawnedCount++;
			if (SpawnedCount >= SplashCountToSpawn)
			{
				break;
			}
		}
	}
}

// 렌더 패스에서 라인을 그리기 위해 호출될 함수
void UParticleRainComponent::RenderRain()
{
	// -------------------------------------------------------------
	// 1. 낙하하는 빗방울 렌더링 (모션 블러 트릭)
	// -------------------------------------------------------------
	// 속도(Velocity) 방향의 반대로 꼬리를 늘려서 선을 생성합니다.
	float RainTailLength = 0.06f; 
	// 진한 파란색 계열로 변경 (R, G, B, A)
	FVector4 RainColor = FVector4(0.2f, 0.4f, 1.0f, 0.7f); 

	for (const auto& Drop : RainPool)
	{
		if (Drop.State != EParticleState::Falling) continue;

		FVector3 Start = Drop.Position;
		FVector3 End = Drop.Position - (Drop.Velocity * RainTailLength);

		FGraphicsManager::Get().DrawLine(Start, End, RainColor);
	}

	// -------------------------------------------------------------
	// 2. 물보라 렌더링
	// -------------------------------------------------------------
	float SplashTailLength = 0.02f;

	for (const auto& Splash : SplashPool)
	{
		if (Splash.State != EParticleState::Splashing) continue;

		FVector3 Start = Splash.Position;
		FVector3 End = Splash.Position - (Splash.Velocity * SplashTailLength);

		// 남은 수명에 비례하여 알파(Alpha) 값을 서서히 0으로 줄임 (페이드 아웃 효과)
		float AlphaRatio = Splash.LifeTime / Splash.MaxLifeTime;
		
		// 물보라는 기존 비 색상보다 약간 밝게 세팅
		FVector4 SplashColor = FVector4(0.4f, 0.6f, 1.0f, AlphaRatio * 0.8f);

		FGraphicsManager::Get().DrawLine(Start, End, SplashColor);
	}
}
