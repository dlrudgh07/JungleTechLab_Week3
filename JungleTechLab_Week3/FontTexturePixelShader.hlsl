cbuffer constants : register(b0)
{
    row_major float4x4 World;
    row_major float4x4 ViewProjection;
    float4 Tint;
    float4 UVTransform;
}

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 texColor = txDiffuse.Sample(samLinear, input.uv);

    //float4 ResultColor = input.color * texColor;

    //ResultColor.a = texColor.r;
    //texColor.a *= Tint.a;
    //return float4(1.0, 0.0, 0.0, 1.0);
    return texColor * Tint;
    //return texColor;
    //return float4(1.0, 0.0, 0.0, 1.0);
}
