#include "../../Common.hlsli"

struct FParticle
{
    float3 Position;
    float3 Velocity;
    float3 Scale;
    float LifeTime;
};

RWStructuredBuffer<FParticle> Particles : register(u0);

//[numthreads(32, 1, 1)]
//void main( uint3 DTid : SV_DispatchThreadID )
//{
//    uint i = DTid.x;
//    Particles[i].LifeTime -= DeltaTime;
    
//    if (Particles[i].LifeTime > 0)
//    {
//        // 속도에 중력 가속도 적용
//        Particles[i].Velocity += float3(0.f, -9.8f, 0.f) * DeltaTime;
//        // 위치 업데이트
//        Particles[i].Position += Particles[i].Velocity * DeltaTime;
//    }
//    else
//    {
//    }
//}

[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint i = DTid.x;
    //Particles[i].LifeTime -= DeltaTime;
    
    if (Particles[i].LifeTime > 0)
    {
        float3 noise = float3(sin(Particles[i].Position.y * 0.5 + Time), cos(Particles[i].Position.z * 0.5 + Time), sin(Particles[i].Position.x * 0.5 + Time));
        Particles[i].Velocity += noise * 0.01f; // 미세한 기류 영향
        //Particles[i].Velocity = noise;
        Particles[i].Velocity *= 0.98f; // 공기 저항 (마찰력)
        
        // 속도에 중력 가속도 적용
        //Particle.Velocity += float3(0.f, -9.8f, 0.f) * DeltaTime;
        // 위치 업데이트
        Particles[i].Position += Particles[i].Velocity * DeltaTime;
    }
    else
    {
    }
}