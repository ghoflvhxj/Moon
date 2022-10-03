cbuffer VS_CBuffer_ABVD : register(b0)
{
    float resolutionWidth;
    float resolutionHeight;
    bool bLight;
};

cbuffer VS_CBuffer_PerTick : register(b1)
{
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;

	// 직교 투영용
    row_major matrix identityMatrix;
    row_major matrix orthographicProjectionMatrix;
    row_major matrix inverseOrthographicProjectionMatrix;
	
    row_major matrix viewMatrixForShadow;
    row_major matrix orthographicForShadow[3];
};