cbuffer constants : register(b0)
{
    row_major float4x4 World;
    row_major float4x4 ViewProjection;
    float4 Tint;
}

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
    float4 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD; 
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD; 
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    output.position = mul(mul(float4(input.position.xyz, 1.0f), World), ViewProjection);
    output.color = float4(lerp(input.color.rgb, Tint.rgb, Tint.a), 1.0f);
    
    output.uv = input.uv; 
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 texColor = txDiffuse.Sample(samLinear, input.uv);

    float4 ResultColor = input.color * texColor;

    ResultColor.a = texColor.r;
    
    return ResultColor;
}
