Texture2D<float4> sceneTexture
    : register(t0, space2);

SamplerState sceneSampler
    : register(s0, space2);

Texture2D<float4> glyphAtlas
    : register(t1, space2);

SamplerState glyphSampler
    : register(s1, space2);

Texture2D<float4> historyTexture
    : register(t2, space2);

SamplerState historySampler
    : register(s2, space2);

cbuffer AsciiData : register(b0, space3)
{
    // x = screen width
    // y = screen height
    // z = cell width
    // w = cell height
    float4 screenAndCell;

    // x = glyph count
    // y = atlas width
    // z = atlas height
    // w = unused
    float4 atlasInfo;

    // x = previous-frame influence
    // y = history valid (0 / 1)
    // z,w = unused
    float4 temporalInfo;
};

struct PSInput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

float GetLuminance(
    float3 color
)
{
    return dot(
        color,
        float3(
            0.2126,
            0.7152,
            0.0722
        )
    );
}

float3 SampleScene(
    float2 pixelPosition,
    float2 screenSize
)
{
    pixelPosition =
        clamp(
            pixelPosition,
            float2(0.5, 0.5),
            screenSize - float2(0.5, 0.5)
        );

    float2 uv =
        pixelPosition /
        screenSize;

    return sceneTexture.SampleLevel(
        sceneSampler,
        uv,
        0.0
    ).rgb;
}

float3 SampleHistory(
    float2 pixelPosition,
    float2 screenSize
)
{
    pixelPosition =
        clamp(
            pixelPosition,
            float2(0.5, 0.5),
            screenSize - float2(0.5, 0.5)
        );

    float2 uv =
        pixelPosition /
        screenSize;

    return historyTexture.SampleLevel(
        historySampler,
        uv,
        0.0
    ).rgb;
}

float SampleGlyphMask(
    int glyphIndex,
    float2 localPosition
)
{
    const float glyphWidth =
        8.0;

    const float glyphHeight =
        8.0;

    float glyphX =
        floor(
            localPosition.x *
            glyphWidth
        );

    float glyphY =
        floor(
            localPosition.y *
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

    return glyphAtlas.SampleLevel(
        glyphSampler,
        atlasUV,
        0.0
    ).r;
}

float4 main(
    PSInput input
) : SV_Target0
{
    float2 screenSize =
        screenAndCell.xy;

    float2 cellSize =
        screenAndCell.zw;

    float2 pixelPosition =
        input.position.xy;

    // =====================================================
    // Current ASCII cell
    // =====================================================

    float2 cellIndex =
        floor(
            pixelPosition /
            cellSize
        );

    float2 cellOrigin =
        cellIndex *
        cellSize;

    float2 cellCenter =
        cellOrigin +
        cellSize *
        0.5;

    float2 offsetX =
        float2(
            cellSize.x * 0.30,
            0.0
        );

    float2 offsetY =
        float2(
            0.0,
            cellSize.y * 0.30
        );

    // =====================================================
    // Current frame samples
    // =====================================================

    float3 centerColor =
        SampleScene(
            cellCenter,
            screenSize
        );

    float3 leftColor =
        SampleScene(
            cellCenter - offsetX,
            screenSize
        );

    float3 rightColor =
        SampleScene(
            cellCenter + offsetX,
            screenSize
        );

    float3 topColor =
        SampleScene(
            cellCenter - offsetY,
            screenSize
        );

    float3 bottomColor =
        SampleScene(
            cellCenter + offsetY,
            screenSize
        );

    float3 averageColor =
        (
            centerColor * 4.0 +
            leftColor +
            rightColor +
            topColor +
            bottomColor
        ) /
        8.0;

    float centerLum =
        GetLuminance(
            centerColor
        );

    float averageLum =
        GetLuminance(
            averageColor
        );

    float leftLum =
        GetLuminance(
            leftColor
        );

    float rightLum =
        GetLuminance(
            rightColor
        );

    float topLum =
        GetLuminance(
            topColor
        );

    float bottomLum =
        GetLuminance(
            bottomColor
        );

    // =====================================================
    // Edge detection
    // =====================================================

    float edgeX =
        abs(
            leftLum -
            rightLum
        );

    float edgeY =
        abs(
            topLum -
            bottomLum
        );

    float edgeCenter =
        max(
            abs(
                centerLum -
                leftLum
            ),
            abs(
                centerLum -
                rightLum
            )
        );

    edgeCenter =
        max(
            edgeCenter,
            abs(
                centerLum -
                topLum
            )
        );

    edgeCenter =
        max(
            edgeCenter,
            abs(
                centerLum -
                bottomLum
            )
        );

    float edge =
        max(
            max(
                edgeX,
                edgeY
            ),
            edgeCenter
        );

    // =====================================================
    // Temporal stabilization
    // =====================================================

    float stableLum =
        averageLum;

    if (
        temporalInfo.y > 0.5
    )
    {
        float3 historyCenter =
            SampleHistory(
                cellCenter,
                screenSize
            );

        float3 historyLeft =
            SampleHistory(
                cellCenter - offsetX,
                screenSize
            );

        float3 historyRight =
            SampleHistory(
                cellCenter + offsetX,
                screenSize
            );

        float3 historyTop =
            SampleHistory(
                cellCenter - offsetY,
                screenSize
            );

        float3 historyBottom =
            SampleHistory(
                cellCenter + offsetY,
                screenSize
            );

        float3 historyAverage =
            (
                historyCenter * 4.0 +
                historyLeft +
                historyRight +
                historyTop +
                historyBottom
            ) /
            8.0;

        float historyLum =
            GetLuminance(
                historyAverage
            );

        stableLum =
            lerp(
                averageLum,
                historyLum,
                saturate(
                    temporalInfo.x
                )
            );
    }

    // =====================================================
    // Dark background removal
    // =====================================================

    const float blackThreshold =
        0.025;

    if (
        averageLum < blackThreshold &&
        edge < 0.018
    )
    {
        return float4(
            0.0,
            0.0,
            0.0,
            1.0
        );
    }

    // =====================================================
    // Stable brightness
    // =====================================================

    float brightness =
        saturate(
            (
                stableLum -
                blackThreshold
            ) *
            2.8
        );

    brightness +=
        edge *
        3.2;

    brightness =
        saturate(
            brightness
        );

    brightness =
        pow(
            brightness,
            0.72
        );

    int glyphCount =
        max(
            (int)atlasInfo.x,
            1
        );

    // =====================================================
    // Smooth glyph transition
    // =====================================================

    float glyphValue =
        brightness *
        (
            glyphCount -
            1
        );

    int glyphA =
        clamp(
            (int)floor(
                glyphValue
            ),
            0,
            glyphCount - 1
        );

    int glyphB =
        min(
            glyphA + 1,
            glyphCount - 1
        );

    float glyphBlend =
        smoothstep(
            0.25,
            0.75,
            frac(
                glyphValue
            )
        );

    float2 localPosition =
        frac(
            pixelPosition /
            cellSize
        );

    float maskA =
        SampleGlyphMask(
            glyphA,
            localPosition
        );

    float maskB =
        SampleGlyphMask(
            glyphB,
            localPosition
        );

    float glyphMask =
        lerp(
            maskA,
            maskB,
            glyphBlend
        );

    // =====================================================
    // Final color uses CURRENT frame, not history.
    // This avoids visible motion trails.
    // =====================================================

    float3 finalObjectColor =
        pow(
            saturate(
                averageColor *
                1.55
            ),
            0.84
        );

    float edgeBoost =
        saturate(
            edge *
            4.0
        );

    finalObjectColor *=
        1.0 +
        edgeBoost *
        0.25;

    float3 finalColor =
        finalObjectColor *
        glyphMask;

    return float4(
        finalColor,
        1.0
    );
}
