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

    // x = fill-glyph count
    // y = full atlas width
    // z = atlas height
    // w = unused
    float4 atlasInfo;

    // x = previous-frame influence
    // y = history valid
    // z,w = unused
    float4 temporalInfo;
};

struct PSInput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

// =========================================================
// Atlas indices
// =========================================================

static const int GLYPH_SPACE      = 0;
static const int GLYPH_DOT        = 1;
static const int GLYPH_COLON      = 2;
static const int GLYPH_MINUS      = 3;
static const int GLYPH_PLUS       = 4;
static const int GLYPH_STAR       = 5;
static const int GLYPH_HASH       = 6;
static const int GLYPH_AT         = 7;

static const int GLYPH_COMMA      = 8;
static const int GLYPH_APOSTROPHE = 9;
static const int GLYPH_SLASH      = 10;
static const int GLYPH_BACKSLASH  = 11;
static const int GLYPH_PIPE       = 12;
static const int GLYPH_TILDE      = 13;
static const int GLYPH_CARET      = 14;
static const int GLYPH_PERCENT    = 15;

static const int GLYPH_LPAREN     = 16;
static const int GLYPH_RPAREN     = 17;
static const int GLYPH_LBRACKET   = 18;
static const int GLYPH_RBRACKET   = 19;
static const int GLYPH_LBRACE     = 20;
static const int GLYPH_RBRACE     = 21;
static const int GLYPH_UNDERSCORE = 22;
static const int GLYPH_EQUALS     = 23;
static const int GLYPH_SEMICOLON  = 24;
static const int GLYPH_BACKTICK   = 25;
static const int GLYPH_LESS       = 26;
static const int GLYPH_GREATER    = 27;
static const int GLYPH_EXCLAM     = 28;
static const int GLYPH_QUESTION   = 29;
static const int GLYPH_AMPERSAND  = 30;
static const int GLYPH_DOLLAR     = 31;

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

    return sceneTexture.SampleLevel(
        sceneSampler,
        pixelPosition / screenSize,
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

    return historyTexture.SampleLevel(
        historySampler,
        pixelPosition / screenSize,
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
            saturate(localPosition.x) *
            glyphWidth
        );

    float glyphY =
        floor(
            saturate(localPosition.y) *
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

float3 MaxColor(
    float3 a,
    float3 b
)
{
    return max(
        a,
        b
    );
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
    // High-detail cell
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
            max(
                cellSize.x * 0.42,
                1.0
            ),
            0.0
        );

    float2 offsetY =
        float2(
            0.0,
            max(
                cellSize.y * 0.42,
                1.0
            )
        );

    float2 diagonalOffset =
        float2(
            offsetX.x,
            offsetY.y
        );

    // =====================================================
    // Nine current-frame samples
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

    float3 topLeftColor =
        SampleScene(
            cellCenter - diagonalOffset,
            screenSize
        );

    float3 topRightColor =
        SampleScene(
            cellCenter +
            float2(
                diagonalOffset.x,
                -diagonalOffset.y
            ),
            screenSize
        );

    float3 bottomLeftColor =
        SampleScene(
            cellCenter +
            float2(
                -diagonalOffset.x,
                diagonalOffset.y
            ),
            screenSize
        );

    float3 bottomRightColor =
        SampleScene(
            cellCenter + diagonalOffset,
            screenSize
        );

    float3 averageColor =
        (
            centerColor * 4.0 +
            leftColor +
            rightColor +
            topColor +
            bottomColor +
            topLeftColor * 0.5 +
            topRightColor * 0.5 +
            bottomLeftColor * 0.5 +
            bottomRightColor * 0.5
        ) /
        10.0;

    // =====================================================
    // Luminance / edge field
    // =====================================================

    float centerLum =
        GetLuminance(centerColor);

    float leftLum =
        GetLuminance(leftColor);

    float rightLum =
        GetLuminance(rightColor);

    float topLum =
        GetLuminance(topColor);

    float bottomLum =
        GetLuminance(bottomColor);

    float topLeftLum =
        GetLuminance(topLeftColor);

    float topRightLum =
        GetLuminance(topRightColor);

    float bottomLeftLum =
        GetLuminance(bottomLeftColor);

    float bottomRightLum =
        GetLuminance(bottomRightColor);

    float averageLum =
        GetLuminance(
            averageColor
        );

    // A luminance change left/right means a VERTICAL contour.
    float edgeX =
        abs(
            leftLum -
            rightLum
        );

    // A luminance change top/bottom means a HORIZONTAL contour.
    float edgeY =
        abs(
            topLum -
            bottomLum
        );

    // NW -> SE gradient => slash contour.
    float edgeDiagSlash =
        abs(
            topLeftLum -
            bottomRightLum
        );

    // NE -> SW gradient => backslash contour.
    float edgeDiagBackslash =
        abs(
            topRightLum -
            bottomLeftLum
        );

    float localMaxLum =
        max(
            centerLum,
            max(
                max(
                    leftLum,
                    rightLum
                ),
                max(
                    topLum,
                    bottomLum
                )
            )
        );

    localMaxLum =
        max(
            localMaxLum,
            max(
                max(
                    topLeftLum,
                    topRightLum
                ),
                max(
                    bottomLeftLum,
                    bottomRightLum
                )
            )
        );

    float localMinLum =
        min(
            centerLum,
            min(
                min(
                    leftLum,
                    rightLum
                ),
                min(
                    topLum,
                    bottomLum
                )
            )
        );

    localMinLum =
        min(
            localMinLum,
            min(
                min(
                    topLeftLum,
                    topRightLum
                ),
                min(
                    bottomLeftLum,
                    bottomRightLum
                )
            )
        );

    float silhouetteRange =
        localMaxLum -
        localMinLum;

    float edge =
        max(
            max(
                edgeX,
                edgeY
            ),
            max(
                edgeDiagSlash,
                edgeDiagBackslash
            )
        );

    // =====================================================
    // Temporal stabilization
    // =====================================================

    float stableLum =
        averageLum;

    if (
        temporalInfo.y >
        0.5
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
    // True black background
    // =====================================================

    const float blackThreshold =
        0.020;

    if (
        localMaxLum <
            blackThreshold &&
        edge <
            0.014
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
    // Fill glyph
    // =====================================================

    float brightness =
        saturate(
            (
                stableLum -
                blackThreshold
            ) *
            3.05
        );

    brightness +=
        edge *
        1.65;

    brightness =
        saturate(
            pow(
                brightness,
                0.78
            )
        );

    int fillGlyphCount =
        max(
            (int)atlasInfo.x,
            1
        );

    float glyphValue =
        brightness *
        (
            fillGlyphCount -
            1
        );

    int glyphA =
        clamp(
            (int)floor(
                glyphValue
            ),
            0,
            fillGlyphCount - 1
        );

    int glyphB =
        min(
            glyphA + 1,
            fillGlyphCount - 1
        );

    float glyphBlend =
        smoothstep(
            0.22,
            0.78,
            frac(
                glyphValue
            )
        );

    float2 localPosition =
        frac(
            pixelPosition /
            cellSize
        );

    float fillMaskA =
        SampleGlyphMask(
            glyphA,
            localPosition
        );

    float fillMaskB =
        SampleGlyphMask(
            glyphB,
            localPosition
        );

    float fillMask =
        lerp(
            fillMaskA,
            fillMaskB,
            glyphBlend
        );

    // =====================================================
    // Outline classification
    //
    // We do not place []{}() everywhere. They are reserved
    // for strong silhouette cells so objects keep readable
    // borders without turning the entire image into noise.
    // =====================================================

    float outlineStrength =
        silhouetteRange *
        0.72 +
        edge *
        0.55;

    bool isOutline =
        localMaxLum >
            0.035 &&
        outlineStrength >
            0.078 &&
        (
            localMinLum <
                localMaxLum *
                0.58 ||
            edge >
                0.115
        );

    int outlineGlyph =
        GLYPH_PIPE;

    if (
        isOutline
    )
    {
        bool cornerLike =
            edgeX >
                0.060 &&
            edgeY >
                0.060;

        if (
            cornerLike &&
            edgeX >=
                max(
                    edgeDiagSlash,
                    edgeDiagBackslash
                )
        )
        {
            // Bright material on the right means this is a LEFT edge.
            bool leftEdge =
                rightLum >
                leftLum;

            if (
                outlineStrength >
                    0.24
            )
            {
                outlineGlyph =
                    leftEdge
                        ? GLYPH_LBRACE
                        : GLYPH_RBRACE;
            }
            else
            {
                outlineGlyph =
                    leftEdge
                        ? GLYPH_LBRACKET
                        : GLYPH_RBRACKET;
            }
        }
        else
        {
            float dominant =
                edgeX;

            outlineGlyph =
                GLYPH_PIPE;

            if (
                edgeY >
                dominant
            )
            {
                dominant =
                    edgeY;

                outlineGlyph =
                    outlineStrength >
                        0.18
                        ? GLYPH_EQUALS
                        : GLYPH_UNDERSCORE;
            }

            if (
                edgeDiagSlash >
                dominant
            )
            {
                dominant =
                    edgeDiagSlash;

                outlineGlyph =
                    GLYPH_SLASH;
            }

            if (
                edgeDiagBackslash >
                dominant
            )
            {
                dominant =
                    edgeDiagBackslash;

                outlineGlyph =
                    GLYPH_BACKSLASH;
            }

            // Vertical silhouettes can become curved/square/brace borders.
            if (
                dominant ==
                    edgeX &&
                edgeX >
                    0.070
            )
            {
                bool leftEdge =
                    rightLum >
                    leftLum;

                if (
                    outlineStrength >
                        0.27
                )
                {
                    outlineGlyph =
                        leftEdge
                            ? GLYPH_LBRACE
                            : GLYPH_RBRACE;
                }
                else if (
                    outlineStrength >
                        0.19
                )
                {
                    outlineGlyph =
                        leftEdge
                            ? GLYPH_LBRACKET
                            : GLYPH_RBRACKET;
                }
                else if (
                    outlineStrength >
                        0.12
                )
                {
                    outlineGlyph =
                        leftEdge
                            ? GLYPH_LPAREN
                            : GLYPH_RPAREN;
                }
                else
                {
                    outlineGlyph =
                        GLYPH_PIPE;
                }
            }
        }
    }

    float glyphMask =
        fillMask;

    if (
        isOutline
    )
    {
        glyphMask =
            SampleGlyphMask(
                outlineGlyph,
                localPosition
            );
    }

    // =====================================================
    // Color
    // =====================================================

    float3 objectColor =
        pow(
            saturate(
                averageColor *
                1.62
            ),
            0.84
        );

    if (
        isOutline
    )
    {
        float3 brightestColor =
            MaxColor(
                centerColor,
                MaxColor(
                    MaxColor(
                        leftColor,
                        rightColor
                    ),
                    MaxColor(
                        topColor,
                        bottomColor
                    )
                )
            );

        objectColor =
            pow(
                saturate(
                    brightestColor *
                    1.82
                ),
                0.80
            );

        objectColor *=
            1.10;
    }
    else
    {
        float edgeBoost =
            saturate(
                edge *
                3.2
            );

        objectColor *=
            1.0 +
            edgeBoost *
            0.16;
    }

    // =====================================================
    // Dark atmospheric haze
    //
    // This is deliberately subtle and screen-space for now.
    // It reduces contrast toward the distant/top area and edges
    // without filling the true-black background.
    // =====================================================

    float2 centeredUV =
        input.uv *
            2.0 -
        1.0;

    float vignette =
        smoothstep(
            0.52,
            1.34,
            length(
                centeredUV *
                float2(
                    0.86,
                    1.0
                )
            )
        );

    float distantHaze =
        saturate(
            (
                0.60 -
                input.uv.y
            ) /
            0.60
        );

    float atmosphere =
        saturate(
            distantHaze *
                0.24 +
            vignette *
                0.16
        );

    float luminanceBeforeFog =
        max(
            GetLuminance(
                objectColor
            ),
            0.001
        );

    float3 coldFogTint =
        float3(
            0.105,
            0.115,
            0.125
        );

    float3 foggedColor =
        lerp(
            objectColor,
            objectColor *
                0.60 +
            coldFogTint *
                luminanceBeforeFog *
                0.42,
            atmosphere
        );

    float3 finalColor =
        foggedColor *
        glyphMask;

    return float4(
        finalColor,
        1.0
    );
}
