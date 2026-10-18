struct VSInput
{
    float3 center     : TEXCOORD0;
    float2 corner     : TEXCOORD1;
    float  glyphIndex : TEXCOORD2;
    float  size       : TEXCOORD3;
    float4 color      : TEXCOORD4;
};

struct VSOutput
{
    float4 position   : SV_Position;
    float2 uv         : TEXCOORD0;
    float  glyphIndex : TEXCOORD1;
    float4 color      : TEXCOORD2;
};

cbuffer GlyphCameraData : register(b0, space1)
{
    float4x4 viewProjection;

    // xyz = camera billboard basis
    float4 cameraRight;
    float4 cameraUp;

    // x = base glyph width in world units
    // y = base glyph height in world units
    // z,w = reserved
    float4 glyphSettings;
};

VSOutput main(
    VSInput input
)
{
    VSOutput output;

    float width =
        glyphSettings.x *
        input.size;

    float height =
        glyphSettings.y *
        input.size;

    float3 worldPosition =
        input.center +
        cameraRight.xyz *
        input.corner.x *
        width +
        cameraUp.xyz *
        input.corner.y *
        height;

    output.position =
        mul(
            viewProjection,
            float4(
                worldPosition,
                1.0
            )
        );

    output.uv =
        input.corner +
        float2(
            0.5,
            0.5
        );

    output.glyphIndex =
        input.glyphIndex;

    output.color =
        input.color;

    return output;
}
