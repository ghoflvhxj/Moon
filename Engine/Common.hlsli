cbuffer CBuffer_ABVD : register(b0)
{
    float resolutionWidth;
    float resolutionHeight;
    bool bLight;
};

cbuffer CBuffer_PerTick : register(b1)
{
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;

	// ���� ������
    row_major matrix identityMatrix;
    row_major matrix orthographicProjectionMatrix;
    row_major matrix inverseOrthographicProjectionMatrix;
    
    float4 lightPos[3];
    row_major matrix lightViewProjMatrix[2];
    float4 cascadeDistance;
};