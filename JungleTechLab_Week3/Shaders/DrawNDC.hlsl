cbuffer GizmoCB : register(b0)
{
    row_major float4x4 ViewRotation;
    float AspectRatio;
    float3 Padding;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

static const float3 AxisDirections[3] =
{
    float3(1, 0, 0), // X
    float3(0, 1, 0), // Y
    float3(0, 0, 1) // Z
};

static const float4 AxisColors[3] =
{
    float4(1, 0, 0, 1), // X: Red
    float4(0, 1, 0, 1), // Y: Green
    float4(0, 0, 1, 1) // Z: Blue
};

VS_OUTPUT mainVS(uint VertexID : SV_VertexID)
{
    VS_OUTPUT Output;

    // 정점 0,1은 X축 / 2,3은 Y축 / 4,5는 Z축
    uint AxisIndex = VertexID / 2;
    uint EndpointIndex = VertexID % 2;

    // 각 선의 첫 번째 정점은 원점, 두 번째 정점은 축 끝점
    float3 LocalPosition =
        EndpointIndex == 0
        ? float3(0, 0, 0)
        : AxisDirections[AxisIndex];

    // 같은 선의 양 끝에 같은 색을 지정
    Output.Color = AxisColors[AxisIndex];

    float3 RotatedPosition =
        mul(float4(LocalPosition, 0.0f), ViewRotation).xyz;

    float Scale = 0.16f;
    RotatedPosition *= Scale;

    // NDC X/Y의 실제 픽셀 비율 보정
    RotatedPosition.x /= AspectRatio;

    // 뷰포트 좌측 상단
    float2 Offset = float2(-0.91f, 0.78f);

    Output.Pos = float4(
        RotatedPosition.x + Offset.x,
        RotatedPosition.y + Offset.y,
        0.0f,
        1.0f);

    return Output;
}

float4 mainPS(VS_OUTPUT Input) : SV_TARGET
{
    return Input.Color;
}
