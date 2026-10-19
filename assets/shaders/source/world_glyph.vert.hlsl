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

    float4 cameraRight;
    float4 cameraUp;

    // x = base width
    // y = base height
    // z = time in seconds
    // w = wind strength
    float4 glyphSettings;
};

VSOutput main(
    VSInput input
)
{
    VSOutput output;

    float time =
        glyphSettings.z;

    float width =
        glyphSettings.x *
        input.size;

    float height =
        glyphSettings.y *
        input.size;

    float3 animatedCenter =
        input.center;

    // =====================================================
    // Grass wind
    //
    // Dedicated grass glyphs are:
    // 9 apostrophe, 10 slash, 11 backslash, 12 pipe.
    // The bottom stays anchored while the upper half sways.
    // =====================================================

    bool grassLike =
        input.glyphIndex >= 9.0 &&
        input.glyphIndex <= 12.0;

    if (
        grassLike
    )
    {
        float topWeight =
            saturate(
                input.corner.y +
                0.5
            );

        float phase =
            input.center.x *
                0.83 +
            input.center.z *
                0.61;

        float wind =
            sin(
                time *
                    1.55 +
                phase
            ) *
            0.060;

        wind +=
            sin(
                time *
                    0.73 +
                phase *
                    1.91
            ) *
            0.025;

        animatedCenter +=
            cameraRight.xyz *
            wind *
            glyphSettings.w *
            topWeight *
            input.size;
    }

    // =====================================================
    // Fire / sparks
    //
    // World fire glyphs use deliberately bright warm colors.
    // They flicker vertically and sideways without needing a
    // particle system yet.
    // =====================================================

    bool fireLike =
        input.color.r > 0.70 &&
        input.color.g > 0.12 &&
        input.color.b < 0.24;

    if (
        fireLike
    )
    {
        float firePhase =
            input.center.x *
                3.1 +
            input.center.z *
                2.7;

        float flicker =
            sin(
                time *
                    7.0 +
                firePhase
            );

        float flicker2 =
            sin(
                time *
                    11.0 +
                firePhase *
                    0.71
            );

        animatedCenter +=
            cameraUp.xyz *
            (
                0.030 +
                flicker *
                    0.026
            ) *
            input.size;

        animatedCenter +=
            cameraRight.xyz *
            flicker2 *
            0.025 *
            input.size;

        width *=
            1.0 +
            flicker2 *
                0.10;

        height *=
            1.0 +
            flicker *
                0.13;
    }

    float3 worldPosition =
        animatedCenter +
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
