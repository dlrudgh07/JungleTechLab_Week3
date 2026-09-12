#include "ResourceManager.h"

#include <wrl/client.h>
#include <d3d11.h>
#include <fstream>

#include "Renderer.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "FPath.h"
#include "Console.h"
#include "StaticMesh.h"
#include "Material.h"
#include "GraphicsManager.h"
#include "Texture.h"
#include "FFontAsset.h"

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

bool FResourceManager::LoadFont_FNTFile(const FString& AssetName, const FString& FilePath)
{
	std::ifstream file(FilePath);
	if (!file.is_open())
	{
		UE_LOG("Cannot open file %s", FilePath.CStr());
		return false;
	}
	
	FFontAsset* NewFont = new FFontAsset();
	NewFont->SetFontName(AssetName);

	std::string rawLine;
	while (std::getline(file, rawLine))
	{
		FString line = std::string_view(rawLine);

		//\r은 있다면 잘라내기
		if (line.EndsWith(std::string("\r")))
		{
			line = line.LeftChop(1);
		}

		//비어있으면
		if (line.Equals(FString("")))
			continue;

		//기본 정보
		if (line.Find(FString("common"), 0) == 0)
		{
			//파싱
			TMap<FString, FString> CharMap = ParseKeyValueLine(line);

			//기본 정보 저장
			NewFont->SetLineHeight(std::stoi(CharMap["lineHeight"]));
			NewFont->SetBaseLine(std::stoi(CharMap["base"]));
			NewFont->SetAtlasWidth(std::stoi(CharMap["scaleW"]));
			NewFont->SetAtlasHeight(std::stoi(CharMap["scaleH"]));
		}
		//페이지 저장
		else if (line.Find(FString("page"), 0) == 0)
		{
			TMap<FString, FString> CharMap = ParseKeyValueLine(line);

			//페이지 인덱스
			int32 PageId = std::stoi(CharMap["id"]);
			FString FileName = CharMap["file"];

			//파일 경로
			FString FilePath = FString("Assets/Fonts/Gulim/").Append(FileName);

			//Page 텍스처 로드
			//나중에 파일 탐색기로 폰트를 임포트 할 수 있게 해야한다.
			if (!LoadTextureFromFile(FileName, FilePath))
			{
				UE_LOG("Font Page Texture Load Failed");
				continue;
			}

			//나중에 찾을 수 있게 인덱스와 이름을 저장
			NewFont->AddPageName(PageId, FileName);
		}
		//문자 데이터 저장
		else if (line.Find(FString("char"), 0) == 0)
		{
			//한줄 파싱
			TMap<FString, FString> CharMap = ParseKeyValueLine(line);

			//chars 항목이면 넘어가기
			if (CharMap.Num() < 2) continue;

			FCharacterInfo info = {};
			info.Id = std::stoi(CharMap["id"]);
			info.X = std::stoi(CharMap["x"]);
			info.Y = std::stoi(CharMap["y"]);
			info.Width = std::stoi(CharMap["width"]);
			info.Height = std::stoi(CharMap["height"]);
			info.XOffset = std::stoi(CharMap["xoffset"]);
			info.YOffset = std::stoi(CharMap["yoffset"]);
			info.XAdvance = std::stoi(CharMap["xadvance"]);
			info.Page = std::stoi(CharMap["page"]);


			NewFont->AddCharInfo(info.Id, info);
		}
		//kerning이 생기면 작업할 것.
		/*else if (line.Find(FString("char"), 0) == 0)
		{

		}*/
	}

	//등록
	RegisterFontAsset(AssetName, NewFont);
	return true;
}

TMap<FString, FString> FResourceManager::ParseKeyValueLine(const FString& line)
{
	TMap<FString, FString> Result;

	int32 i = 0;
	int32 len = line.Len();

	FString remaining = line;

	while (remaining.Len() > 0)
	{
		//공백 인덱스 찾기
		int32 SpaceIndex = remaining.Find(FString(" "), 0);

		//공백이 없다면 한 글자, 있다면 공백 왼쪽으로 가져오기
		FString Token = (SpaceIndex == -1) ? remaining : remaining.Left(SpaceIndex);
		//한글자였다면 빈칸으로 두어 while 탈출, 아니라면 다음 검사를 위해 공백 오른쪽 문자열로 할당
		remaining = (SpaceIndex == -1) ? FString("") : remaining.RightChop(SpaceIndex + 1);

		//공백 왼쪽 문자열에 = 가 없다면 common char와 같은 타입이다.
		int32 EqualIndex = Token.Find(FString("="), 0);
		if (EqualIndex == -1) continue;

		//key, value 할당
		FString key = Token.Left(EqualIndex);
		FString value = Token.RightChop(EqualIndex + 1);

		//value가 page의 file일 경우 따옴표를 제거해야 함.
		//""만 해도 len이 2다.
		if (value.Len() >= 2 && value.StartsWith(FString("\"")) && value.EndsWith(FString("\"")))
		{
			value = value.Mid(1, value.Len() - 2);
		}

		Result.Add(key, value);
	}

	return Result;
}

void FResourceManager::RegisterFontAsset(const std::string& Name, FFontAsset* Font)
{
	assert(Font != nullptr && "Cannot register a null Font!");
	FontAssetMap[Name] = Font;
}

FFontAsset* FResourceManager::GetFontAsset(const std::string& Name) const
{
	auto it = FontAssetMap.find(Name);
	if (it != FontAssetMap.end())
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

	for (auto& Pair : FontAssetMap)
	{
		delete Pair.second;
	}
	FontAssetMap.clear();
}


bool FResourceManager::LoadTextureFromFile(const std::string& AssetName, const std::string& FilePath)
{
	// 1. 이미 등록된 이름이면 로드하지 않음
	if (TextureMap.find(AssetName) != TextureMap.end())
	{
		UE_LOG("이미 등록된 %s 텍스처입니다.", AssetName);
		return false;
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
		return false;
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

	return true;
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
	if (!LoadTextureFromFile("CrateTexture", "crate.jpg")) UE_LOG("CrateTexture 로드 실패");
	
	if (!LoadTextureFromFile("FontTexture", "Assets/DDS/FontAtlas.dds")) UE_LOG("FontTexture 로드 실패");

	//굴림 폰트 아틀라스 로드
	if (!LoadFont_FNTFile("Gulim", "Assets/Fonts/Gulim/Gulim.fnt")) UE_LOG("굴림체 폰트 로드 실패");

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

	// 3-3. Ascii 아틀라스 폰트 머티리얼
	UMaterial* FotnMaterial = FObjectFactory::ConstructObject<UMaterial>();
	FotnMaterial->TintColor = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
	FotnMaterial->BaseTexture = GetTexture("FontTexture");
	RegisterMaterial("FotnMaterial", FotnMaterial);


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
