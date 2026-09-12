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

void FResourceManager::InitializeDefaultAssets(FGraphicsManager* GraphicsManager)
{
	// ==========================================
	// [1] 그래픽스 버퍼 생성 
	// ==========================================
	FBuffer* CubeBuffer = GraphicsManager->CreateBuffer(Cube_vertices, sizeof(Cube_vertices));
	FBuffer* QuadBuffer = GraphicsManager->CreateBuffer(Quad_vertices, sizeof(Quad_vertices));
	FBuffer* SphereBuffer = GraphicsManager->CreateBuffer(Sphere_vertices, sizeof(Sphere_vertices));
	FBuffer* GizmoArrowBuffer = GraphicsManager->CreateBuffer(GizmoArrow_vertices, sizeof(GizmoArrow_vertices));
	FBuffer* CircleBuffer = GraphicsManager->CreateBuffer(Circle_vertices, sizeof(Circle_vertices));

	// ==========================================
	// [2] 텍스처 에셋 로드 및 등록
	// ==========================================
	// (LoadTextureFromFile 내부에서 파일 읽기 + UTexture 생성 + RegisterTexture 까지 한 번에 해줌)
	LoadTextureFromFile("CrateTexture", "C:/Users/JUNGLE/Desktop/GameEngine/Week3/JungleTechLab_Week3/JungleTechLab_Week3/crate.jpg");

	// ==========================================
	// [3] 머티리얼 에셋 생성 및 등록
	// ==========================================
	// 3-1. 디폴트 화이트 머티리얼 (단색 큐브용)
	UMaterial* DefaultMaterial = FObjectFactory::ConstructObject<UMaterial>();
	DefaultMaterial->TintColor = FVector4(1.0f, 1.0f, 1.0f, 0.0f);
	DefaultMaterial->BaseTexture = GetDefaultWhiteTexture(); // 매니저에 내장된 디폴트 화이트 UTexture
	RegisterMaterial("DefaultMaterial", DefaultMaterial);

	// 3-2. 나무 상자 머티리얼
	UMaterial* CrateMaterial = FObjectFactory::ConstructObject<UMaterial>();
	CrateMaterial->TintColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	CrateMaterial->BaseTexture = GetTexture("CrateTexture");
	RegisterMaterial("CrateMaterial", CrateMaterial);


	// ==========================================
	// [4] 스태틱 메쉬 에셋 생성 및 등록
	// ==========================================
	uint32 VertCount = sizeof(Cube_vertices) / sizeof(FVertexSimple);

	// 4-1. 기본 큐브 메쉬
	UStaticMesh* CubeMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	CubeMesh->VertexBuffer = CubeBuffer;
	CubeMesh->StaticMaterials.Add(GetMaterial("DefaultMaterial"));
	for (uint32 i = 0; i < VertCount; ++i) CubeMesh->CPUVertices.emplace_back(Cube_vertices[i]);
	RegisterStaticMesh("Cube", CubeMesh);

	// 4-2. 나무 상자 메쉬 (모양은 큐브 버퍼를 똑같이 쓰고, 머티리얼만 갈아끼움)
	UStaticMesh* CrateMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	CrateMesh->VertexBuffer = CubeBuffer;
	CrateMesh->StaticMaterials.Add(GetMaterial("CrateMaterial"));
	for (uint32 i = 0; i < VertCount; ++i) CrateMesh->CPUVertices.emplace_back(Cube_vertices[i]);
	RegisterStaticMesh("Crate", CrateMesh);

	// 4-3. 스피어 메쉬
	UStaticMesh* SphereMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	SphereMesh->VertexBuffer = SphereBuffer;
	SphereMesh->StaticMaterials.Add(GetMaterial("DefaultMaterial"));
	VertCount = sizeof(Sphere_vertices) / sizeof(FVertexSimple);
	for (uint32 i = 0; i < VertCount; ++i) SphereMesh->CPUVertices.emplace_back(Sphere_vertices[i]);
	RegisterStaticMesh("Sphere", SphereMesh);

	// 4-4. 기즈모 메쉬
	UStaticMesh* GizmoArrowMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	GizmoArrowMesh->VertexBuffer = GizmoArrowBuffer;
	GizmoArrowMesh->StaticMaterials.Add(GetMaterial("DefaultMaterial"));
	VertCount = sizeof(GizmoArrow_vertices) / sizeof(FVertexSimple);
	for (uint32 i = 0; i < VertCount; ++i) GizmoArrowMesh->CPUVertices.emplace_back(GizmoArrow_vertices[i]);
	RegisterStaticMesh("GizmoArrow", GizmoArrowMesh);

	UStaticMesh* CircleMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	CircleMesh->VertexBuffer = CircleBuffer;
	CircleMesh->StaticMaterials.Add(GetMaterial("DefaultMaterial"));
	VertCount = sizeof(Circle_vertices) / sizeof(FVertexSimple);
	for (uint32 i = 0; i < VertCount; ++i) CircleMesh->CPUVertices.emplace_back(Circle_vertices[i]);
	RegisterStaticMesh("Circle", CircleMesh);

	// 4-5. 쿼드 메쉬
	UStaticMesh* QuadMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	QuadMesh->VertexBuffer = QuadBuffer;
	QuadMesh->StaticMaterials.Add(GetMaterial("DefaultMaterial"));
	VertCount = sizeof(Quad_vertices) / sizeof(FVertexSimple);
	for (uint32 i = 0; i < VertCount; ++i) QuadMesh->CPUVertices.emplace_back(Quad_vertices[i]);
	RegisterStaticMesh("Quad", QuadMesh);
}
