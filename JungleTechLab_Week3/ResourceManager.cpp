#include "ResourceManager.h"

#include <wrl/client.h>
#include <d3d11.h>

#include "Renderer.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "FPath.h"
#include "Console.h"
#include "StaticMesh.h"
#include "Material.h"
#include "GraphicsManager.h"
#include "Texture.h"

#include "Cube.h"
#include "Sphere.h"
#include "Circle.h"
#include "Quad.h"
#include "Triangle.h"
#include "GizmoArrow.h"


FResourceManager::FResourceManager()
{
}

FResourceManager::~FResourceManager()
{
	ClearAll();
}


void FResourceManager::Initialize(ID3D11Device* InputDevice, ID3D11DeviceContext* InputContext)
{
	Device = InputDevice;
	DeviceContext = InputContext;
}



void FResourceManager::RegisterStaticMesh(const std::string& Name, UStaticMesh* Mesh)
{
	assert(Mesh != nullptr && "Cannot register a null StaticMesh!");
	StaticMeshMap[Name] = Mesh;
}

UStaticMesh* FResourceManager::GetStaticMesh(const std::string& Name) const
{
	auto it = StaticMeshMap.find(Name);
	if (it != StaticMeshMap.end())
	{
		return it->second;
	}
	return nullptr; // 에셋을 못 찾았을 경우
}

void FResourceManager::RegisterMaterial(const std::string& Name, UMaterial* Material)
{
	assert(Material != nullptr && "Cannot register a null Material!");
	MaterialMap[Name] = Material;
}

UMaterial* FResourceManager::GetMaterial(const std::string& Name) const
{
	auto it = MaterialMap.find(Name);
	if (it != MaterialMap.end())
	{
		return it->second;
	}
	return nullptr;
}

void FResourceManager::RegisterTexture(const std::string& Name, UTexture* Texture)
{
	assert(Texture != nullptr && "Cannot register a null Texture!");
	TextureMap[Name] = Texture;
}

UTexture* FResourceManager::GetTexture(const std::string& Name) const
{
	auto it = TextureMap.find(Name);
	if (it != TextureMap.end())
	{
		return it->second;
	}
	return nullptr;
}

void FResourceManager::ClearAll()
{
	// 별도의 GC나 메모리 풀로 관리된다면
	// 여기서 delete를 직접 호출하는 코드는 지워져야함

	for (auto& Pair : StaticMeshMap)
	{
		delete Pair.second;
	}
	StaticMeshMap.clear();

	for (auto& Pair : MaterialMap)
	{
		delete Pair.second;
	}
	MaterialMap.clear();

	for (auto& Pair : TextureMap)
	{
		delete Pair.second;
	}
	TextureMap.clear();
}


void FResourceManager::LoadTextureFromFile(const std::string& AssetName, const std::string& FilePath)
{
	// 1. 이미 등록된 이름이면 로드하지 않음
	if (TextureMap.find(AssetName) != TextureMap.end())
	{
		return;
	}

	// 2. 새로운 UTexture 껍데기 생성
	UTexture* NewTexture = FObjectFactory::ConstructObject<UTexture>();

	FString FileExtension = FPaths::GetExtension(FilePath).ToLower();
	std::wstring WideFilePath = FPaths::StringToWString(FilePath);

	Microsoft::WRL::ComPtr<ID3D11Resource> RawResource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RawSRV;

	HRESULT hr = S_OK;

	// 3. 파일 로드 (WIC / DDS)
	if (FileExtension == "dds")
	{
		hr = DirectX::CreateDDSTextureFromFile(Device.Get(), WideFilePath.c_str(), RawResource.GetAddressOf(), RawSRV.GetAddressOf());
	}
	else
	{
		hr = DirectX::CreateWICTextureFromFile(Device.Get(), DeviceContext.Get(), WideFilePath.c_str(), RawResource.GetAddressOf(), RawSRV.GetAddressOf());
	}

	if (FAILED(hr))
	{
		UE_LOG("Failed to load texture: %s", FilePath.c_str());
		delete NewTexture;
		return;
	}

	// 4. GPU 데이터(SRV)를 FTextureResource 래퍼로 묶어서 UTexture에 연결
	NewTexture->Resource = new FTextureResource{ RawSRV };

	// 5. 텍스처 해상도 추출
	Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture2D;
	if (SUCCEEDED(RawResource.As(&Texture2D)))
	{
		D3D11_TEXTURE2D_DESC Desc;
		Texture2D->GetDesc(&Desc);
		NewTexture->Width = Desc.Width;
		NewTexture->Height = Desc.Height;
	}

	// 6. 캐시에 등록
	RegisterTexture(AssetName, NewTexture);
}


UTexture* FResourceManager::GetDefaultWhiteTexture() const
{
	return GetTexture("DefaultWhite");
}


void FResourceManager::CreateAndRegisterStaticMesh(
	class FGraphicsManager* GraphicsManager,
	const std::string& MeshName,
	const FVertexSimple* Vertices, uint32 NumVerts,
	const uint32* Indices, uint32 NumIndices,
	const std::string& MaterialName)
{
	UStaticMesh* Mesh = FObjectFactory::ConstructObject<UStaticMesh>(
		GraphicsManager->CreateVertexBuffer(Vertices, sizeof(FVertexSimple) * NumVerts),
		Vertices, NumVerts,
		GraphicsManager->CreateIndexBuffer(Indices, sizeof(uint32) * NumIndices),
		Indices, NumIndices
	);

	Mesh->StaticMaterials.Add(GetMaterial(MaterialName));
	RegisterStaticMesh(MeshName, Mesh);
}


void FResourceManager::InitializeDefaultAssets(FGraphicsManager* GraphicsManager)
{
	// ==========================================
	// [1] 텍스처 에셋 로드 및 등록
	// ==========================================
	// (LoadTextureFromFile 내부에서 파일 읽기 + UTexture 생성 + RegisterTexture 까지 한 번에 해줌)
	LoadTextureFromFile("CrateTexture", "C:/Users/JUNGLE/Desktop/GameEngine/Week3/JungleTechLab_Week3/JungleTechLab_Week3/crate.jpg");

	LoadTextureFromFile("FontTexture", "Assets/DDS/FontAtlas.dds");

	// ==========================================
	// [2] 머티리얼 에셋 생성 및 등록
	// ==========================================
	// 2-1. 디폴트 화이트 머티리얼
	UMaterial* DefaultMaterial = FObjectFactory::ConstructObject<UMaterial>();
	DefaultMaterial->TintColor = FVector4(1.0f, 1.0f, 1.0f, 0.0f);
	DefaultMaterial->BaseTexture = GetDefaultWhiteTexture(); // 매니저에 내장된 디폴트 화이트 UTexture
	RegisterMaterial("DefaultMaterial", DefaultMaterial);

	// 2-2. 나무 상자 머티리얼
	UMaterial* CrateMaterial = FObjectFactory::ConstructObject<UMaterial>();
	CrateMaterial->TintColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	CrateMaterial->BaseTexture = GetTexture("CrateTexture");
	RegisterMaterial("CrateMaterial", CrateMaterial);

	// 2-3. Ascii 아틀라스 폰트 머티리얼
	UMaterial* FotnMaterial = FObjectFactory::ConstructObject<UMaterial>();
	FotnMaterial->TintColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	FotnMaterial->BaseTexture = GetTexture("FontTexture");
	RegisterMaterial("FontMaterial", FotnMaterial);


	// ==========================================
	// [3] 스태틱 메쉬 에셋 생성 및 등록 (Initialize()을 이용한 방식)
	// todo: 이건 추후에 asset import 기능 추가되면 런타임에서 가져오도록 변경해야함
	// ==========================================
	//

	// 3-1. 기본 큐브 메쉬
	CreateAndRegisterStaticMesh(GraphicsManager, "Cube", Cube_vertices, Cube_indices, "DefaultMaterial");

	// 3-2. 나무 상자 메쉬 (모양은 큐브 버퍼를 똑같이 쓰고, 머티리얼만 갈아끼움)
	CreateAndRegisterStaticMesh(GraphicsManager, "Crate", Cube_vertices, Cube_indices, "CrateMaterial");

	// 3-3. 스피어 메쉬
	std::vector<uint32> SphereDummyIndices;
	uint32 VertexCount = sizeof(Sphere_vertices) / sizeof(FVertexSimple);
	for (uint32 CurrentCount = 0; CurrentCount < VertexCount; ++CurrentCount)
		SphereDummyIndices.push_back(CurrentCount);
	CreateAndRegisterStaticMesh(GraphicsManager, "Sphere", Sphere_vertices, VertexCount, SphereDummyIndices.data(), static_cast<uint32>(SphereDummyIndices.size()), "DefaultMaterial");

	// 3-4. 기즈모 메쉬
	std::vector<uint32> GizmoArrowDummyIndices;
	VertexCount = sizeof(GizmoArrow_vertices) / sizeof(FVertexSimple);
	for (uint32 CurrentCount = 0, EndCount = sizeof(GizmoArrow_vertices) / sizeof(FVertexSimple); CurrentCount < EndCount; ++CurrentCount)
		GizmoArrowDummyIndices.push_back(CurrentCount);
	CreateAndRegisterStaticMesh(GraphicsManager, "GizmoArrow", GizmoArrow_vertices, VertexCount, GizmoArrowDummyIndices.data(), static_cast<uint32>(GizmoArrowDummyIndices.size()), "DefaultMaterial");

	std::vector<uint32> CircleDummyIndices;
	VertexCount = sizeof(Circle_vertices) / sizeof(FVertexSimple);
	for (uint32 CurrentCount = 0, EndCount = sizeof(Circle_vertices) / sizeof(FVertexSimple); CurrentCount < EndCount; ++CurrentCount)
		CircleDummyIndices.push_back(CurrentCount);
	CreateAndRegisterStaticMesh(GraphicsManager, "Circle", Circle_vertices, VertexCount, CircleDummyIndices.data(), static_cast<uint32>(CircleDummyIndices.size()), "DefaultMaterial");

	// 3-5. 쿼드 메쉬
	CreateAndRegisterStaticMesh(GraphicsManager, "Quad", Quad_vertices, Quad_indices, "DefaultMaterial");
}
