#include "../../VSCommon.hlsli"

struct FParticle
{
    float3 Position;
    float3 Velocity;
    float3 Scale;
    float LifeTime;
};

StructuredBuffer<FParticle> Particles : register(t0);

VertexOut main(VertexIn vIn, uint InstanceID : SV_InstanceID)
{
    VertexOut vOut = (VertexOut)0;
    int ID = InstanceID;
    FParticle Particle = Particles[ID];
    
    float4 ViewPos = mul(float4(Particle.Position, 1.f), viewMatrix);
    ViewPos.xy += vIn.pos.xy * Particle.Scale.xy;
    
    vOut.pos = mul(ViewPos, projectionMatrix);
    
    //vOut.pos = LocalToProj(float4(vIn.pos.xyz + Particles[ID].Position.xyz, 1.f));
    vOut.uv = vIn.uv;
    vOut.normal = vIn.normal;
    vOut.tangent = vIn.tangent;
    vOut.binormal = vIn.binormal;
    
    return vOut;
}