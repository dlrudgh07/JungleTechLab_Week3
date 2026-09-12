#pragma once
#include <string>
#include <unordered_map>
#include <assert.h>
#include "Core.h"
#include <d3d11.h>
#include <wrl/client.h>

class UStaticMesh;
class UMaterial;
class UTexture;
struct FVertexSimple;
struct FTextureResource;
class FGraphicsManager; // 버퍼 생성을 위해 전방 선언

class FResourceManager
{
public:
	FResourceManager();
	~FResourceManager();

	void Initialize(ID3D11Device* InDevice, ID3D11DeviceContext* InContext);

	// 엔진 기본 에셋 세팅
	void InitializeDefaultAssets(FGraphicsManager* GraphicsManager);

	void RegisterStaticMesh(const std::string& Name, UStaticMesh* Mesh);
	UStaticMesh* GetStaticMesh(const std::string& Name) const;

	void RegisterMaterial(const std::string& Name, UMaterial* Material);
	UMaterial* GetMaterial(const std::string& Name) const;

	void RegisterTexture(const std::string& Name, UTexture* Texture);
	UTexture* GetTexture(const std::string& Name) const;

	// todo : 나중에 FStirng으로 변환 필요
	void LoadTextureFromFile(const std::string& AssetName, const std::string& FilePath);
	UTexture* GetDefaultWhiteTexture() const;

	void ClearAll();

private:
	void CreateDefaultWhiteTexture();

	Microsoft::WRL::ComPtr<ID3D11Device>        Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext;

	std::unordered_map<std::string, UStaticMesh*> StaticMeshMap;
	std::unordered_map<std::string, UMaterial*> MaterialMap;
	std::unordered_map<std::string, UTexture*> TextureMap;
};
