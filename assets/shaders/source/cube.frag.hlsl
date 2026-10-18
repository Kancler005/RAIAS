struct PSInput
{
    float4 position    : SV_Position;
    float3 worldNormal : TEXCOORD0;
    float3 color       : TEXCOORD1;
};

cbuffer LightData : register(b0, space3)
{
    float4 lightDirection;
    float4 lightColor;
    float4 lightingParams;
};

float4 main(PSInput input) : SV_Target0
{
    float3 normal =
        normalize(
            input.worldNormal
        );

    float3 directionToLight =
        normalize(
            -lightDirection.xyz
        );

    float diffuse =
        max(
            dot(
                normal,
                directionToLight
            ),
            0.0
        );

    float ambientStrength =
        lightingParams.x;

    float diffuseStrength =
        lightingParams.y;

    float3 ambient =
        input.color *
        ambientStrength;

    float3 diffuseColor =
        input.color *
        lightColor.rgb *
        diffuse *
        diffuseStrength;

    float3 finalColor =
        saturate(
            ambient +
            diffuseColor
        );

    return float4(
        finalColor,
        1.0
    );
}