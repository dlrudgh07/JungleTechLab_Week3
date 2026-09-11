// ShaderW0.hlsl

// texture mapping
Texture2D DiffuseMap : register(t0);
SamplerState Sampler : register(s0);



cbuffer constants : register(b0)
{
    row_major float4x4 World;
    row_major float4x4 ViewProjection;
    float4 Tint; // rgb = 덧입힐 색, a = 섞는 비율(0 이면 정점 색 그대로)
}

struct VS_INPUT
{
    float4 position : POSITION; // Input position from vertex buffer
    float4 color : COLOR; // Input color from vertex buffer

    float2 uv : TEXCOORD; // texture mapping
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
    float4 color : COLOR; // Color to pass to the pixel shader

    float2 uv : TEXCOORD; // texture mapping
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    output.position = mul(mul(float4(input.position.xyz, 1.0f), World), ViewProjection);
    
    // 큐브 면 색을 Tint 쪽으로 섞어서, 같은 정점 버퍼로도 오브젝트를 구분할 수 있게 한다
    output.color = float4(lerp(input.color.rgb, Tint.rgb, Tint.a), 1.0f);

    output.uv = input.uv; // texture mapping
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // texture mapping
    float4 TextureColor = DiffuseMap.Sample(Sampler, input.uv);

    
    // Output the color directly
    return TextureColor;
}
