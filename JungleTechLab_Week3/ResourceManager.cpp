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
#include "IniParser.h"


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
	if (Name.empty() || !Mesh)
		throw std::runtime_error("Invalid asset registration");
	for (const auto& [Key, Value] : StaticMeshMap)
	{
		if (Key == Name && Value == Mesh)
			return;
		if (Key == Name || Value == Mesh)
			throw std::runtime_error("Duplicate asset registration");
	}
	StaticMeshMap.emplace(Name, Mesh);
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
	if (Name.empty() || !Material)
		throw std::runtime_error("Invalid asset registration");
	for (const auto& [Key, Value] : MaterialMap)
	{
		if (Key == Name && Value == Material)
			return;
		if (Key == Name || Value == Material)
			throw std::runtime_error("Duplicate asset registration");
	}
	MaterialMap.emplace(Name, Material);
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
	if (Name.empty() || !Texture)
		throw std::runtime_error("Invalid asset registration");
	for (const auto& [Key, Value] : TextureMap)
	{
		if (Key == Name && Value == Texture)
			return;
		if (Key == Name || Value == Texture)
			throw std::runtime_error("Duplicate asset registration");
	}
	TextureMap.emplace(Name, Texture);
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
	NewTexture->SourcePath = FString(FilePath);

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
	// [1] 텍스처 에셋 로드 및 등록
	// ==========================================
	// (LoadTextureFromFile 내부에서 파일 읽기 + UTexture 생성 + RegisterTexture 까지 한 번에 해줌)
	LoadTextureFromFile("CrateTexture", "./Assets/FireAnimationTexture.png");
	LoadTextureFromFile("FontTexture", "./Assets/DDS/FontAtlas.dds");
	LoadTextureFromFile("FireTexture", "./Assets/FireAnimationTexture.png");

	// ==========================================
	// [2] 머티리얼 에셋 생성 및 등록
	// ==========================================
	// 디폴트 화이트 머티리얼 (단색 큐브용)
	UMaterial* DefaultMaterial = FObjectFactory::ConstructObject<UMaterial>();
	DefaultMaterial->TintColor = FVector4(1.0f, 1.0f, 1.0f, 0.0f);
	DefaultMaterial->BaseTexture = GetDefaultWhiteTexture(); // 매니저에 내장된 디폴트 화이트 UTexture
	RegisterMaterial("DefaultMaterial", DefaultMaterial);

	// 나무 상자 머티리얼
	UMaterial* CrateMaterial = FObjectFactory::ConstructObject<UMaterial>();
	CrateMaterial->TintColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	CrateMaterial->BaseTexture = GetTexture("CrateTexture");
	RegisterMaterial("CrateMaterial", CrateMaterial);

	// 3-3. Ascii 아틀라스 폰트 머티리얼
	UMaterial* FontTexture = FObjectFactory::ConstructObject<UMaterial>();
	FontTexture->TintColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	FontTexture->BaseTexture = GetTexture("FontTexture");
	RegisterMaterial("FontTexture", FontTexture);

	// 3-4. suvUV 머티리얼 
	UMaterial* SubUVMaterial = FObjectFactory::ConstructObject<UMaterial>();
	SubUVMaterial->TintColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	SubUVMaterial->BaseTexture = GetTexture("FireTexture");
	RegisterMaterial("SubUVMaterial", SubUVMaterial);

	// ==========================================
	// [3] 스태틱 메쉬 에셋 생성 및 등록
	// todo - 지금은 하드코딩으로 버퍼를 만들고 잇지만, 나중에는 이것들을 import 기능을 통해 동적으로 처리할수있도록해야함
	// todo - 순서가 반복되는데, 이걸 별도의 템플릿이나 함수로 빼야할거같음
	// /		다만 import기능이 어떤식으로 구현될지 몰라서, 일단 이대로 두었음
	// ==========================================
	// 기본 큐브 메쉬
	UStaticMesh* CubeMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	FBuffer* CubeBuffer = GraphicsManager->CreateBuffer(Cube_vertices, sizeof(Cube_vertices), CubeMesh->CPUVertices, CubeMesh->CPUIndices);
	CubeMesh->VertexBuffer = CubeBuffer;
	CubeMesh->StaticMaterials.Add(GetMaterial("DefaultMaterial"));
	CubeMesh->Initialize();
	RegisterStaticMesh("Cube", CubeMesh);

	// 나무 상자 메쉬 (모양은 큐브 버퍼를 똑같이 쓰고, 머티리얼만 갈아끼움)
	UStaticMesh* CrateMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	// cube buffer와 동일하지만, "asset은 자신의 buffer를 독립적으로 갖는다" 를 원칙으로 진행하여, double free 되는 것을 방지한다.
	FBuffer* CrateBuffer = GraphicsManager->CreateBuffer(Cube_vertices, sizeof(Cube_vertices), CrateMesh->CPUVertices, CrateMesh->CPUIndices);
	CrateMesh->VertexBuffer = CrateBuffer;
	CrateMesh->StaticMaterials.Add(GetMaterial("CrateMaterial"));
	CrateMesh->Initialize();
	RegisterStaticMesh("Crate", CrateMesh);

	// 스피어 메쉬
	UStaticMesh* SphereMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	FBuffer* SphereBuffer = GraphicsManager->CreateBuffer(Sphere_vertices, sizeof(Sphere_vertices), SphereMesh->CPUVertices, SphereMesh->CPUIndices);
	SphereMesh->VertexBuffer = SphereBuffer;
	SphereMesh->StaticMaterials.Add(GetMaterial("DefaultMaterial"));
	SphereMesh->Initialize();
	RegisterStaticMesh("Sphere", SphereMesh);

	// 기즈모 메쉬
	UStaticMesh* GizmoArrowMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	FBuffer* GizmoArrowBuffer = GraphicsManager->CreateBuffer(GizmoArrow_vertices, sizeof(GizmoArrow_vertices), GizmoArrowMesh->CPUVertices, GizmoArrowMesh->CPUIndices);
	GizmoArrowMesh->VertexBuffer = GizmoArrowBuffer;
	GizmoArrowMesh->StaticMaterials.Add(GetMaterial("DefaultMaterial"));
	GizmoArrowMesh->Initialize();
	RegisterStaticMesh("GizmoArrow", GizmoArrowMesh);

	UStaticMesh* CircleMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	FBuffer* CircleBuffer = GraphicsManager->CreateBuffer(Circle_vertices, sizeof(Circle_vertices), CircleMesh->CPUVertices, CircleMesh->CPUIndices);
	CircleMesh->VertexBuffer = CircleBuffer;
	CircleMesh->StaticMaterials.Add(GetMaterial("DefaultMaterial"));
	CircleMesh->Initialize();
	RegisterStaticMesh("Circle", CircleMesh);

	// 쿼드 메쉬
	UStaticMesh* QuadMesh = FObjectFactory::ConstructObject<UStaticMesh>();
	FBuffer* QuadBuffer = GraphicsManager->CreateBuffer(Quad_vertices, sizeof(Quad_vertices), QuadMesh->CPUVertices, QuadMesh->CPUIndices);
	QuadMesh->VertexBuffer = QuadBuffer;
	QuadMesh->StaticMaterials.Add(GetMaterial("DefaultMaterial"));
	QuadMesh->Initialize();
	RegisterStaticMesh("Quad", QuadMesh);
}

#include "SceneSerialization.h"
#include <unordered_set>
void FResourceManager::InitializeForLoad(FResourceManager& Target) const
{
	Target.Initialize(Device.Get(), DeviceContext.Get());
}
void FResourceManager::SwapAssets(FResourceManager& Other)
{
	StaticMeshMap.swap(Other.StaticMeshMap);
	MaterialMap.swap(Other.MaterialMap);
	TextureMap.swap(Other.TextureMap);
}
void FResourceManager::SerializeAssets(json::JSON& Out) const
{
	Out = json::JSON::Make(json::JSON::Class::Array);
	std::unordered_set<const UObject*> Saved;
	auto Save = [&](const auto& Map) {
		for (const auto& [Name, Asset] : Map)
		{
			if (!Asset || !Saved.insert(Asset).second)
				throw std::runtime_error("Asset registered more than once");
			json::JSON Data;
			Asset->SerializeClass(Data);
			Data["AssetName"] = Name;
			Out.append(Data);
		}
	};
	Save(TextureMap);
	Save(MaterialMap);
	Save(StaticMeshMap);
	// References to unregistered resources must not silently produce broken scenes.
	auto Check = [&](const UObject* O) {
		if (O && !Saved.contains(O))
			throw std::runtime_error("Register referenced asset before saving scene");
	};
	for (const auto& [N, M] : MaterialMap)
		Check(M->BaseTexture);
	for (const auto& [N, M] : StaticMeshMap)
		for (auto* Material : M->StaticMaterials)
			Check(Material);
}
void FResourceManager::DeserializeAssets(const json::JSON& In)
{
	std::unique_ptr<FSceneLoadScope> OwnScope;
	if (!FSceneLoadScope::Current)
		OwnScope = std::make_unique<FSceneLoadScope>();
	if (!StaticMeshMap.empty() || !MaterialMap.empty() || !TextureMap.empty())
		throw std::runtime_error("Asset load requires an empty resource manager");
	if (In.JSONType() != json::JSON::Class::Array)
		throw std::runtime_error("Assets requires array");
	std::vector<std::pair<UObject*, const json::JSON*>> Pending;
	for (const auto& Data : In.ArrayRange())
	{
		const auto Name = Data.at("AssetName").ToString();
		if (Name.empty())
			throw std::runtime_error("Empty asset name");
		auto Object = PreloadObject<UObject>(Data);
		auto* Raw = Object.get();
		if (Raw->IsA<UTexture>())
		{
			if (TextureMap.contains(Name))
				throw std::runtime_error("Duplicate texture name");
			RegisterTexture(Name, static_cast<UTexture*>(Raw));
		}
		else if (Raw->IsA<UMaterial>())
		{
			if (MaterialMap.contains(Name))
				throw std::runtime_error("Duplicate material name");
			RegisterMaterial(Name, static_cast<UMaterial*>(Raw));
		}
		else if (Raw->IsA<UStaticMesh>())
		{
			if (StaticMeshMap.contains(Name))
				throw std::runtime_error("Duplicate mesh name");
			RegisterStaticMesh(Name, static_cast<UStaticMesh*>(Raw));
		}
		else
			throw std::runtime_error("Unsupported asset class");
		Object.release();
		Pending.emplace_back(Raw, &Data);
	}
	for (auto [Object, Data] : Pending)
		Object->DeserializeClass(*Data);
	for (auto& [Name, Texture] : TextureMap)
	{
		if (!Device)
			throw std::runtime_error("Texture loading requires device");
		auto Path = FPaths::StringToWString(Texture->SourcePath.CStr());
		Microsoft::WRL::ComPtr<ID3D11Resource> Resource;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV;
		HRESULT Result;
		if (FPaths::GetExtension(Texture->SourcePath.CStr()).ToLower() == "dds")
			Result = DirectX::CreateDDSTextureFromFile(Device.Get(), Path.c_str(), Resource.GetAddressOf(),
													   SRV.GetAddressOf());
		else
			Result = DirectX::CreateWICTextureFromFile(Device.Get(), DeviceContext.Get(), Path.c_str(),
													   Resource.GetAddressOf(), SRV.GetAddressOf());
		if (FAILED(Result))
			throw std::runtime_error("Failed to restore texture: " + std::string(Texture->SourcePath.CStr()));
		Texture->Resource = new FTextureResource{SRV};
		Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture2D;
		if (SUCCEEDED(Resource.As(&Texture2D)))
		{
			D3D11_TEXTURE2D_DESC Desc{};
			Texture2D->GetDesc(&Desc);
			Texture->Width = static_cast<int32>(Desc.Width);
			Texture->Height = static_cast<int32>(Desc.Height);
		}
	}
	for (auto& [Name, Mesh] : StaticMeshMap)
	{
		if (Mesh->CPUVertices.empty() && Mesh->CPUIndices.empty())
			continue;
		if (!Device || Mesh->CPUVertices.empty() || Mesh->CPUIndices.empty())
			throw std::runtime_error("Invalid mesh buffers");
		auto Buffer = std::make_unique<FBuffer>();
		auto Create = [&](const void* Data, size_t Bytes, UINT Bind, ID3D11Buffer** Out) {
			if (Bytes > UINT_MAX)
				throw std::runtime_error("Mesh buffer too large");
			D3D11_BUFFER_DESC Desc{};
			Desc.ByteWidth = static_cast<UINT>(Bytes);
			Desc.Usage = D3D11_USAGE_DEFAULT;
			Desc.BindFlags = Bind;
			D3D11_SUBRESOURCE_DATA Initial{};
			Initial.pSysMem = Data;
			if (FAILED(Device->CreateBuffer(&Desc, &Initial, Out)))
				throw std::runtime_error("Failed to restore mesh buffer");
		};
		Create(Mesh->CPUVertices.data(), Mesh->CPUVertices.size() * sizeof(FVertexSimple), D3D11_BIND_VERTEX_BUFFER,
			   Buffer->VertexBuffer.GetAddressOf());
		Create(Mesh->CPUIndices.data(), Mesh->CPUIndices.size() * sizeof(uint32), D3D11_BIND_INDEX_BUFFER,
			   Buffer->IndexBuffer.GetAddressOf());
		Buffer->NumVertices = static_cast<uint32>(Mesh->CPUVertices.size());
		Buffer->NumIndices = static_cast<uint32>(Mesh->CPUIndices.size());
		Mesh->VertexBuffer = Buffer.release();
	}
}
