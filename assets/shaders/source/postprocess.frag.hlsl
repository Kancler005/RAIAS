Texture2D<float4> sceneTexture
    : register(t0, space2);

SamplerState sceneSampler
    : register(s0, space2);

Texture2D<float4> glyphAtlas
    : register(t1, space2);

SamplerState glyphSampler
    : register(s1, space2);

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
};

struct PSInput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

// =========================================================
// Luminance
// =========================================================

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

// =========================================================
// Sample scene by pixel coordinate
// =========================================================

float3 SampleScene(
    float2 pixelPosition,
    float2 screenSize
)
{
    pixelPosition =
        clamp(
            pixelPosition,
            float2(
                0.5,
                0.5
            ),
            screenSize -
            float2(
                0.5,
                0.5
            )
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

// =========================================================
// Main
// =========================================================

float4 main(
    PSInput input
) : SV_Target0
{
    const float glyphWidth =
        8.0;

    const float glyphHeight =
        8.0;

    float2 screenSize =
        screenAndCell.xy;

    float2 cellSize =
        screenAndCell.zw;

    float2 pixelPosition =
        input.position.xy;

    // =====================================================
    // ASCII CELL
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

    // =====================================================
    // SAMPLE MULTIPLE POINTS
    // =====================================================

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

    // =====================================================
    // AVERAGE COLOR
    // =====================================================

    float3 averageColor =
        (
            centerColor * 4.0 +
            leftColor +
            rightColor +
            topColor +
            bottomColor
        ) /
        8.0;

    // =====================================================
    // LUMINANCE
    // =====================================================

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
    // EDGE DETECTION
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
    // REMOVE VERY DARK BACKGROUND
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
    // ASCII INTENSITY
    // =====================================================

    float brightness =
        saturate(
            (
                averageLum -
                blackThreshold
            ) *
            2.8
        );

    // Contours receive stronger glyphs.

    brightness +=
        edge *
        3.5;

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

    int glyphIndex =
        (int)round(
            brightness *
            (
                glyphCount -
                1
            )
        );

    glyphIndex =
        clamp(
            glyphIndex,
            0,
            glyphCount - 1
        );

    // =====================================================
    // GLYPH POSITION
    // =====================================================

    float2 localPosition =
        frac(
            pixelPosition /
            cellSize
        );

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

    // =====================================================
    // GLYPH ATLAS
    // =====================================================

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

    float glyphMask =
        glyphAtlas.SampleLevel(
            glyphSampler,
            atlasUV,
            0.0
        ).r;

    // =====================================================
    // COLOR
    // =====================================================

    float3 finalObjectColor =
        averageColor;

    // Slightly brighten colors.

    finalObjectColor =
        pow(
            saturate(
                finalObjectColor *
                1.65
            ),
            0.82
        );

    // Edge cells become brighter.

    float edgeBoost =
        saturate(
            edge *
            5.0
        );

    finalObjectColor *=
        1.0 +
        edgeBoost *
        0.35;

    // =====================================================
    // PURE ASCII
    // Background inside each cell is black.
    // =====================================================

    float3 finalColor =
        finalObjectColor *
        glyphMask;

    return float4(
        finalColor,
        1.0
    );
}