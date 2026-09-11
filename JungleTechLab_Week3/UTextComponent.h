#pragma once
#include "PrimitiveComponent.h"
#include "Core.h"

class FFontAsset;

class UTextComponent :
    public UPrimitiveComponent
{
public:
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
	FString Text;
	FFontAsset* FontAsset;
};

