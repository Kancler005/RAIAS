struct VSInput
{
    float3 position : TEXCOORD0;
    float3 color    : TEXCOORD1;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 color    : TEXCOORD0;
};

cbuffer CameraData : register(b0, space1)
{
    float4x4 mvp;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    output.position = mul(
        mvp,
        float4(input.position, 1.0)
    );

    output.color = input.color;

    return output;
}