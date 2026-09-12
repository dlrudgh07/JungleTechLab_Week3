#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "Matrix.h"
#include "Vector.h"
#include "RenderInfo.h"
#include <wrl/client.h>
#include "Console.h"
#include "DDSTextureLoader.h"

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// 1. Define the triangle vertices
struct FVertexSimple
{
	float x, y, z;    // Position (12 byte)
	float r, g, b, a; // Color    (16 byte)
	float u, v;       //  UV 좌표 (8 byte)

	FVector GetPosition() const { return FVector(x, y, z); }
};

struct FConstants
{
	FMatrix World; //Model
	FMatrix ViewProjection;
	FVector4 Tint;          // rgb = 색, a = 섞는 비율
};


class URenderer
{
public:
	// todo 이거 comptr로 다 변경해야함
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;

    ID3D11Texture2D* FrameBuffer = nullptr;
    ID3D11RenderTargetView* FrameBufferRTV = nullptr;
	ID3D11RasterizerState* RasterizerState[2] = {};
    ID3D11Buffer* ConstantBuffer = nullptr;
	ID3D11Texture2D* DepthStencilBuffer = nullptr;			// 실제 깊이값이 저장될 메모리
	ID3D11DepthStencilView* DepthStencilView = nullptr;		// 그 메모리를 "출력 대상"으로 보는 뷰
	ID3D11DepthStencilState* DepthStencilState = nullptr;	// 깊이 테스트용 상태
	ID3D11DepthStencilState* StencilMarkState = nullptr;	// 스텐실에 1 마킹용 상태
	ID3D11DepthStencilState* StencilOutlineState = nullptr; // 아웃라인 그리기용

	ID3D11BlendState* NoColorWriteBlendState = nullptr;		// 스텐실만 찍고 색은 쓰지 않는 상태
	ID3D11BlendState* AlphaBlendState; //텍스쳐 알파값 제거용

    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
    D3D11_VIEWPORT ViewportInfo;
    ID3D11VertexShader* SimpleVertexShader;
    ID3D11PixelShader* SimplePixelShader;
    ID3D11InputLayout* SimpleInputLayout;

	ID3D11ShaderResourceView* UUIDTextureView;
	D3D11_SAMPLER_DESC UUIDSamplerInfo;
	ID3D11SamplerState* UUIDSamplerState;

	// 매 프레임 내용이 바뀌는 선분용. 메시 버퍼와 달리 IMMUTABLE이 아니라 DYNAMIC이다
	ID3D11Buffer* LineVertexBuffer = nullptr;
	uint32 LineVertexCapacity = 0;

	// 매 프레임 갯수가 바뀌는 UUID용. 메시 버퍼와 달리 IMMUTABLE이 아니라 DYNAMIC이다
	ID3D11Buffer* UUIDVertexBuffer = nullptr;
	uint32 UUIDVertexCapacity = 0;


	// texture mapping
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DefaultWhiteTextureSRV = nullptr;
	ID3D11ShaderResourceView* CurrentSRVCache[8] = { nullptr };		 // size는 변경가능 - todo: 아마 최대 사이즈 체크 해야할듯함
	ID3D11SamplerState* CurrentSamplerCache[8] = { nullptr };			// size는 변경가능 - todo: 아마 최대 사이즈 체크 해야할듯함
	void CreateSamplerState();
	void BindTexture(uint32 Slot, ID3D11ShaderResourceView* SRV);
	void BindSampler(uint32 Slot, ID3D11SamplerState* Sampler);
	ID3D11ShaderResourceView* FontTextureSRV;
	void CreateDefaultWhiteTexture();
	


    unsigned int Stride;

public:

	//create
	void Create(HWND hWindow);
	void CreateDeviceAndSwapChain(HWND hWindow);
	void CreateShader();
	void CreateFrameBuffer();
	ID3D11Buffer* CreateVertexBuffer(const FVertexSimple* vertices, const UINT ByteWidth);
	ID3D11Buffer* CreateIndexBuffer(const void* indices, const UINT ByteWidth);
	void CreateLineVertexBuffer(uint32 maxVertices);
	void CreateUUIDVertexBuffer(uint32 maxVertices);
	void CreateRasterizerState();
	void CreateConstantBuffer();
	void CreateDepthStencilBuffer(UINT width, UINT height);

	void CreateDepthStencilState();
	void CreateStencilMarkState();
	void CreateStencilOutlineState();
	void CreateNoColorWriteBlendState();
	void CreateAlphaBlendState();

	//release
	void Release();
	void ReleaseDeviceAndSwapChain();
	void ReleaseShader();
	void ReleaseFrameBuffer();
	void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer);
	void ReleaseLineVertexBuffer();
	void ReleaseUUIDVertexBuffer();
	void ReleaseRasterizerState();
	void ReleaseConstantBuffer();
	void ReleaseDepthStencilBuffer();
	void ReleaseDepthStencilState();
	void ReleaseBlendState();
	void ReleaseUUIDSampleState();

	//Update
	void RSUpdateState();

	//Rendering
	void Prepare(bool bWireFrame);
	void PrepareShader();
	void PrepareTextureShader();
	void UpdateConstant(FMatrix world, FMatrix viewProjection, FVector4 tint = FVector4(0, 0, 0, 0));
	void RenderPrimitive(ID3D11Buffer* pVertexBuffer, ID3D11Buffer* pIndexBuffer, UINT indexCount);
	void RenderLines(const FVertexSimple* vertices, uint32 numVertices);
	void RenderUUID(const FVertexSimple* vertices, uint32 numVertices);
	bool ReAllocateUUIDVertexBuffer(uint32 RequestSize);
	void RenderHighlight(ID3D11Buffer* pVertexBuffer, ID3D11Buffer* pIndexBuffer, uint32 indexCount, FMatrix mViewProjectionMatrix, FMatrix Outline, const FRenderInfo& RI);
	void SwapBuffer();


	//Initialize
	void ClearDepth();
	void ClearTextureCache();
    //=============================================
	//해상도 변경 시 호출
	//void OnResize(UINT Width, UINT Height);
	void OnResize(UINT width, UINT height, float viewportWidth, float viewportHeight);
};
