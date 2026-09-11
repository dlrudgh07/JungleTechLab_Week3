#pragma once

// todo: 지금은 그냥 stl을 사용하지만, 나중에는 custom allocator를 구현해서, 그걸로 변경해서 사용해야함
#include <string>
#include <unordered_map>
#include <assert.h>


class UStaticMesh;
class UMaterial;
class UTexture;
struct FVertexSimple;

class FResourceManager
{
public:
	FResourceManager();
	~FResourceManager();

	// ==========================================
	// Static Mesh 관리
	// ==========================================
	void RegisterStaticMesh(const std::string& Name, UStaticMesh* Mesh);
	UStaticMesh* GetStaticMesh(const std::string& Name) const;

	// ==========================================
	// Material 관리
	// ==========================================
	void RegisterMaterial(const std::string& Name, UMaterial* Material);
	UMaterial* GetMaterial(const std::string& Name) const;

	// ==========================================
	// Texture 관리
	// ==========================================
	void RegisterTexture(const std::string& Name, UTexture* Texture);
	UTexture* GetTexture(const std::string& Name) const;

	// 모든 에셋을 메모리에서 해제 (엔진 종료 시)
	void ClearAll();

private:
	// 에셋 딕셔너리 (실제 UE의 Asset Registry 역할)
	std::unordered_map<std::string, UStaticMesh*> StaticMeshMap;
	std::unordered_map<std::string, UMaterial*> MaterialMap;
	std::unordered_map<std::string, UTexture*> TextureMap;
};
