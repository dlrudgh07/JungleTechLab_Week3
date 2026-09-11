#include "Core.h"


#include <Windows.h>

#include <unordered_map>
#include <memory>
#include <wrl/client.h>
#include <d3d11.h>

#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "FTexture.h"

class FTextureManager
{
	// TODO : this doesn't have to be singleton
public:
	static FTextureManager& GetManager()
	{
		static FTextureManager TextureManager;
		return TextureManager;
	}
	FTextureManager(const FTextureManager&) = delete;
	FTextureManager& operator=(const FTextureManager&) = delete;
	void Initialize(ID3D11Device* InputDevice, ID3D11DeviceContext* InputDeviceContext);
private:
	FTextureManager() = default;
	~FTextureManager() = default;


private:
	Microsoft::WRL::ComPtr<ID3D11Device>        Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;

	// 파일 경로(문자열)를 키값으로 텍스처를 저장하는 캐시 맵
	// shared pointer는 느리기때문에, unique pointer를 사용하고 get으로 넘김
	// 
	// 대신에 "객체의 생명주기(Lifecycle) 동기화"라는 규칙이 엄격하게 지켜져야함
	// 해제 순서 강제 : 무언가를 그리는 모든 오브젝트(몬스터, 맵, UI)는 반드시 ResourceManager가 초기화(Clear)되기 전에 먼저 소멸되어야 합니다.
	//씬(Scene) 단위 관리 : 씬이 바뀔 때, 1) 모든 게임 오브젝트를 먼저 싹 다 지우고 -> 2) ResourceManager의 리소스 캐시를 비우는 순서를 엔진 루프에 하드코딩해 두어야 합니다.

	// 이거 FString으로 쓰려면
	// 이걸 Fstring으로 쓰려면 Fstring용 hash 코드 짜면 됨
	// TODO -> 일단 이대로 진행
	std::unordered_map<std::string, std::unique_ptr<FTexture>> TextureMap;


	void CreateDefaultWhiteTexture();

public:
	FTexture* LoadTexture(const FString& FilePath);
	FTexture* GetDefaultWhiteTexture()
	{
		// LoadTexture에 하드코딩된 키값을 넘겨서 캐시에서 바로 가져옴
		return LoadTexture("DefaultWhite");
	}
};
