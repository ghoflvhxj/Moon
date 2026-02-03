#include "../../VSCommon.hlsli"

VertexOut_SimpleTex main(VertexIn vIn)
{
    VertexOut_SimpleTex vOut = (VertexOut_SimpleTex)0;

    vOut.pos = LocalToProj(vIn.pos);
    vOut.color = vIn.color;
    vOut.uv = vIn.uv;
	
	return vOut;
}