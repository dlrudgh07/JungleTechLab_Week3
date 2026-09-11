#include "UTextComponent.h"
#include "FResourceManager.h"

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
