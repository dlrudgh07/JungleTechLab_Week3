#include "UTextComponent.h"
#include "FResourceManager.h"
#include "FFontAsset.h"
#include "FTexture.h"

UTextComponent::UTextComponent()
{
	SetFontAsset("");
}

UTextComponent::~UTextComponent()
{
}

void UTextComponent::Initialize()
{
	SetFontAsset("");
	FCharDataInfo CharInfo = {};
	bool isFirstChar = true;
	int32 PageIndex = 0;
	for (uint32 ch : Text)
	{
		//한글자만, 다른 글자는 다른 UText 객체 생성.
		if (isFirstChar)
		{
			const FCharacterInfo* CInfo = FontAsset->FindCharInfo(ch);
			if (!CInfo) continue;

			CharInfo.CharX = CInfo->X;
			CharInfo.CharY = CInfo->Y;
			CharInfo.CharWidth = CInfo->Width;
			CharInfo.CharHeight = CInfo->Height;
			CharInfo.AtlasWidth = FontAsset->GetAtlasWidth();
			CharInfo.AtlasHeight = FontAsset->GetAtlasHeight();
			PageIndex = CInfo->Page;
			isFirstChar = false; 
		}
	}
	Initialize(FVector(0.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 0.f), CharInfo, FontAsset->GetPageTexture(PageIndex)->GetSRV());
}

void UTextComponent::Initialize(FVector location, FRotator rotation, FVector scale3D, FCharDataInfo CInfo, ID3D11ShaderResourceView* pSRV)
{
	SetFontAsset("");
	UPrimitiveComponent::Initialize(EPrimitive::EP_Quad, location, rotation, scale3D, CInfo, pSRV);
}

const FString& UTextComponent::GetText() const
{
	return Text;
}

void UTextComponent::SetText(FString& text)
{
	Text = text;
}

void UTextComponent::SetFontAsset(const FString& fontName)
{
	FontAsset = FResourceManager::Get().FindFont(0);
}

void UTextComponent::BuildTextQuads()
{

}
