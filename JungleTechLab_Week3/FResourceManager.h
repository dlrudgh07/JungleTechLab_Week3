#pragma once
#include "Core.h"
#include "TMap.h"

class FFontAsset;
class ID3D11Device;
class FTexture;

class FResourceManager
{
public:
	//싱글톤
	static FResourceManager& Get()
	{
		static FResourceManager Instance;
		return Instance;
	}

	//이동, 복사 금지
	FResourceManager(const FResourceManager&) = delete;
	FResourceManager& operator=(const FResourceManager&) = delete;
	FResourceManager(FResourceManager&&) = delete;
	FResourceManager& operator=(FResourceManager&&) = delete;

	//초기화
	void Initialize();

	//폰트 임포트
	FFontAsset* ImportFont(const FString& FontFilePath, bool bWriteManifest = true);

	//텍스처 로드
	FTexture* LoadTexture(const FString& FilePath);
	FTexture* FindTexture(const int32 Key)const;

	//폰트 찾기. 지금은 FName이 없어서 int으로 둔다.
	FFontAsset* FindFont(const int32& Key) const;

	//Device 받기
	void SetDevice(ID3D11Device* device);

	//명시적 정리
	void Release();


private:
	FResourceManager() = default;
	~FResourceManager();

	//폰트 에셋들 FName으로 관리하도록 변경 필요
	TMap<int32, FFontAsset*> Fonts;

	//텍스처들 관리. FName이 생긴다면 key 바꾸기
	TMap<int32, FTexture*> Textures;

	//매니페스트 만들기

	//Device. 절대 소유하진 않습니다.
	ID3D11Device* Device = nullptr;

	//임시 키 : FName이 생긴다면 관리.
	int32 Key = 0;
};

