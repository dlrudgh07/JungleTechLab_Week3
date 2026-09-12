#pragma once
#include "PrimitiveComponent.h"
#include "Core.h"
#include <d3d11.h>

class FFontAsset;

class UTextComponent :
    public UPrimitiveComponent
{
REFLECT_CLASS(UTextComponent, UPrimitiveComponent)
public:
	UTextComponent();
	virtual ~UTextComponent();

	void Initialize();
	void Initialize(FVector location, FRotator rotation, FVector scale3D, FCharDataInfo CInfo, ID3D11ShaderResourceView* pSRV);

	//저장된 글자 반환
	const FString& GetText()const;
	//어떤 글자를 렌더할지
	void SetText(FString& text);

	//필요한가?
	//FFontAsset* GetFontAsset()const;
	//폰드 에셋 설정
	void SetFontAsset(const FString& fontName);

	//쿼드 생성
	void BuildTextQuads();

private:
	FString Text = "가";
	FFontAsset* FontAsset;

	//일단 여기에 생성.
	TMap<int32, ID3D11Buffer*> VertexBuffers;
};
