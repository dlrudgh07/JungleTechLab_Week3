#pragma once
#include "PrimitiveComponent.h"
#include "Core.h"
#include "TMap.h"
#include "TArray.h"

class FFontAsset;
struct FBuffer;

class UTextComponent :
    public UPrimitiveComponent
{
REFLECT_CLASS(UTextComponent, UPrimitiveComponent)
public:
	UTextComponent();
	virtual ~UTextComponent();

	// factory 에서 필요함
	void Initialize() {};

	virtual void Update(TArray<FRenderInfo>* OutRenderInfos, float DeltaTime) override;	

	//저장된 글자 반환
	const std::wstring& GetText()const;
	//어떤 글자를 렌더할지
	void SetText(const std::wstring& text);

	//폰드 에셋 설정
	FFontAsset* GetFontAsset()const;
	void SetFontAsset(FFontAsset* Font);

	//쿼드 생성
	void BuildTextQuads();

	// 핵심: 렌더러로 데이터 넘기기
	void AddRenderInfos(TArray<FRenderInfo>* outRenderInfos) const override;

	void Release();

private:
	std::wstring Text = L"가";

	FFontAsset* FontAsset = nullptr;

	TMap<int32, FBuffer*> PageBuffers;

	//더티 상태일 때만 버텍스 버퍼를 업데이트합니다.
	bool bDirty = true;

	FVector4 Color = FVector4(1.f, 1.f, 1.f, 1.f);
};
