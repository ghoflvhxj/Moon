#include "VSCommon.hlsli"

VertexOut main(VertexIn vIn)
{
	//VertexOut vOut;

	//matrix worldView = mul(worldMatrix, viewMatrix);
	//matrix worldViewProj = mul(worldView, projectionMatrix);

	//matrix boneTransform = keyFrameMatrices[vIn.blendIndex[0]];
	//float4 animatedPos = mul(float4(vIn.pos, 1.f), boneTransform);
	//vOut.pos		= mul(float4(animatedPos.x, animatedPos.y, animatedPos.z, 1.f), worldViewProj);
 //   vOut.worldPos	= mul(float4(animatedPos.x, animatedPos.y, animatedPos.z, 1.f), worldMatrix).xyz;
	//vOut.uv			= vIn.uv;
	//vOut.normal		= mul(float4(vIn.normal, 0.f), worldView).xyz;
	//vOut.tangent	= mul(float4(vIn.tangent, 0.f), worldView).xyz;
	//vOut.binormal	= mul(float4(vIn.binormal, 0.f), worldView).xyz;

	//return vOut;
	
    VertexOut vOut;

    // ���� ���� ĳ��
    matrix worldView = mul(worldMatrix, viewMatrix);
    matrix worldViewProj = mul(worldView, projectionMatrix);
    //float4 localPos = float4(vIn.pos, 1.f);
    //matrix boneTransform = keyFrameMatrices[vIn.blendIndex[0]];
    float4 animatedPos = mul(float4(vIn.pos, 1.f), keyFrameMatrices[vIn.blendIndex[0]]);
    
    // ��ȯ�� ��ǥ ���
    vOut.pos = mul(animatedPos, worldViewProj);
    vOut.worldPos = mul(animatedPos, worldMatrix).xyz;

    // ����, ź��Ʈ, ���̳�� ���
    float4 normal = float4(vIn.normal, 0.f);
    float4 tangent = float4(vIn.tangent, 0.f);
    float4 binormal = float4(vIn.binormal, 0.f);

    vOut.normal = mul(normal, worldView).xyz;
    vOut.tangent = mul(tangent, worldView).xyz;
    vOut.binormal = mul(binormal, worldView).xyz;

    // �ؽ�ó ��ǥ ����
    vOut.uv = vIn.uv;

    return vOut;
}
