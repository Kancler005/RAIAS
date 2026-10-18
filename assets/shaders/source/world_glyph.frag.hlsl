Texture2D<float4> glyphAtlas
    : register(t0, space2);

SamplerState glyphSampler
    : register(s0, space2);

cbuffer GlyphAtlasData : register(b0, space3)
{
    // x = glyph count
    // y = atlas width
    // z = atlas height
    // w = reserved
    float4 atlasInfo;
};

struct PSInput
{
    float4 position   : SV_Position;
    float2 uv         : TEXCOORD0;
    float  glyphIndex : TEXCOORD1;
    float4 color      : TEXCOORD2;
};

float4 main(
    PSInput input
) : SV_Target0
{
    const float glyphWidth =
        8.0;

    const float glyphHeight =
        8.0;

    float2 localUV =
        saturate(
            input.uv
        );

    float glyphX =
        floor(
            localUV.x *
            glyphWidth
        );

    float glyphY =
        floor(
            localUV.y *
            glyphHeight
        );

    glyphX =
        clamp(
            glyphX,
            0.0,
            glyphWidth - 1.0
        );

    glyphY =
        clamp(
            glyphY,
            0.0,
            glyphHeight - 1.0
        );

    float glyphIndex =
        clamp(
            floor(
                input.glyphIndex +
                0.5
            ),
            0.0,
            atlasInfo.x - 1.0
        );

    float atlasX =
        glyphIndex *
        glyphWidth +
        glyphX;

    float atlasY =
        glyphY;

    float2 atlasUV =
        (
            float2(
                atlasX,
                atlasY
            ) +
            0.5
        ) /
        atlasInfo.yz;

    float mask =
        glyphAtlas.SampleLevel(
            glyphSampler,
            atlasUV,
            0.0
        ).r;

    clip(
        mask -
        0.5
    );

    return float4(
        input.color.rgb,
        1.0
    );
}
