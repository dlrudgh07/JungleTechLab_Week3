#pragma once
#include "Vector.h"

// 1. 에셋 정보 (텍스처당 1개씩 존재)
struct FSubUVInfo
{
	int Cols = 1;
	int Rows = 1;
	int TotalFrames = 1;    // 4x4 배열이라도 실제 이미지는 14프레임일 수 있음
	float FrameRate = 30.0f; // 초당 재생할 프레임 수
};

// 2. 파티클이나 컴포넌트가 들고 있을 재생기 (개별 오브젝트마다 존재)
class FSubUVAnimator
{
public:
	FSubUVAnimator()
	{
		Initialize(3,3, 9, 10);		// todo 지금은 하드코딩으로 초기값 세팅하지만, 이건 따로 빼야함
	}
	void Initialize(int InputCols, int InputRows, int InputTotalFrames, float InputFrameRate, bool InputbLoop = true)
	{
		Info.Cols = InputCols;
		Info.Rows = InputRows;
		Info.TotalFrames = InputTotalFrames;
		Info.FrameRate = InputFrameRate;

		ElapsedTime = 0.0f;
		CurrentFrame = 0;
		bLoop = InputbLoop;

		bIsPlaying = true;
	}

	// 매 프레임(Tick)마다 호출해서 프레임을 넘겨주는 함수
	void Update(float DeltaTime)
	{
		if (!bIsPlaying || Info.TotalFrames <= 1) return;

		ElapsedTime += DeltaTime;
		float TimePerFrame = 1.0f / Info.FrameRate;

		// 프레임 넘어갈 시간이 되었는가?
		while (ElapsedTime >= TimePerFrame)
		{
			ElapsedTime -= TimePerFrame;
			CurrentFrame++;

			// 마지막 프레임 도달 시 처리
			if (CurrentFrame >= Info.TotalFrames)
			{
				if (bLoop) {
					CurrentFrame = 0; // 루프
				}
				else {
					CurrentFrame = Info.TotalFrames - 1; // 마지막 프레임 고정
					bIsPlaying = false;
				}
			}
		}
	}

	// 핵심: 렌더러에게 넘겨줄 (ScaleU, ScaleV, OffsetU, OffsetV) 계산
	FVector4 GetUVScaleOffset() const
	{
		float ScaleU = 1.0f / Info.Cols;
		float ScaleV = 1.0f / Info.Rows;

		int ColIndex = CurrentFrame % Info.Cols;
		int RowIndex = CurrentFrame / Info.Cols;

		float OffsetU = ColIndex * ScaleU;
		float OffsetV = RowIndex * ScaleV;

		return FVector4(ScaleU, ScaleV, OffsetU, OffsetV);
	}

	void SetIsPlaying(bool bInputPlaying)
	{
		bIsPlaying = bInputPlaying;
	}

private:
	FSubUVInfo Info;

	// 재생 상태 관리
	float ElapsedTime = 0.0f;
	int CurrentFrame = 0;
	bool bLoop = false;
	bool bIsPlaying = false;
};
