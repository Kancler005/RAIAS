struct VSInput
{
    float3 position : TEXCOORD0;
    float3 normal   : TEXCOORD1;
};

struct VSOutput
{
    float4 position    : SV_Position;
    float3 worldNormal : TEXCOORD0;
    float3 color       : TEXCOORD1;
};

cbuffer TransformData : register(b0, space1)
{
    float4x4 mvp;
    float4x4 normalMatrix;
};

cbuffer ObjectData : register(b1, space1)
{
    float4 objectColor;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    output.position =
        mul(
            mvp,
            float4(input.position, 1.0)
        );

    output.worldNormal =
        normalize(
            mul(
                normalMatrix,
                float4(input.normal, 0.0)
            ).xyz
        );

    output.color =
        objectColor.rgb;

    return output;
}