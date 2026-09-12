// ShaderW0.hlsl
cbuffer constants : register(b0)
{
    row_major float4x4 World;
    row_major float4x4 ViewProjection;
    float4 Tint; // rgb = 덧입힐 색, a = 섞는 비율(0 이면 정점 색 그대로)
    
	//텍스처 내 글자 위치
    int CharX, CharY;
	//글자의 크기
    int CharWidth, CharHeight;

	//아틀라스 Width, Height
    int AtlasWidth = 0;
    int AtlasHeight = 0;

    float2 pad;
}

Texture2D FontAtlas : register(t0);
SamplerState FontSampler : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
    float4 color : COLOR; // Color to pass to the pixel shader
    float2 uv : TEXCOORD0;
};

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Output the color directly
    //return input.color;
    //float2 CalUV = float2(0, 0);
    //CalUV.x = ((float) CharX + input.uv.x * (float) CharWidth) / (float) AtlasWidth;
    //CalUV.y = ((float) CharY + input.uv.y * (float) CharHeight) / (float) AtlasHeight;
    
    ////float glyphAlpha = FontAtlas.Sample(FontSampler, CalUV).a;
    ////float4 glyphAlpha = FontAtlas.Sample(FontSampler, CalUV);
    //float glyphAlpha = FontAtlas.Sample(FontSampler, CalUV).a;
    //float4 tex = FontAtlas.Sample(FontSampler, input.uv);
    
    ////float4 glyphAlpha = FontAtlas.Sample(FontSampler, CalUV).a;

    //// 텍스처에서는 모양(알파)만 가져오고, 색상은 셰이더 상수(TintColor)로 입힘
    //// -> 같은 아틀라스로 텍스트 색을 자유롭게 바꿀 수 있는 이유가 바로 이 구조
    ////float finalAlpha = glyphAlpha * TintColor.a;

    //// 알파가 0에 가까운 픽셀(글자 바깥 여백)은 굳이 블렌딩 연산까지 갈 필요 없이 버림
    //// -> Alpha Test 성격의 최적화 (완전히 투명한 픽셀의 오버드로우 방지)
    ////clip(glyphAlpha - 0.001f);
    ////return float4(glyphAlpha, glyphAlpha, glyphAlpha, glyphAlpha);
    //return tex;


    float2 CalUV;
    CalUV.x = (44.0 + input.uv.x * 21.0f) / 256.0;
    CalUV.y = (59.0 + input.uv.y * 23.0f) / 256.0;

    float glyphAlpha = FontAtlas.Sample(FontSampler, CalUV).a;
    return float4(glyphAlpha, glyphAlpha, glyphAlpha, glyphAlpha);
    //return float4(input.uv.x, input.uv.y, 0, 1);
    //return FontAtlas.Sample(FontSampler, input.uv);
    //return input.color;
    //return glyphAlpha;
};
