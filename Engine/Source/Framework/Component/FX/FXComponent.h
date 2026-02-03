#pragma once

#include "Include.h"
#include "PrimitiveComponent.h"
#include "Module/Graphic/StructuredBuffer.h"

#include "Mesh/StaticMesh/StaticMesh.h"
#include "Material.h"

struct FEmitterData
{
    uint32 MinNum = 1;
    uint32 MaxNum = 1;

    float MinScale = 1.f;
    float MaxScale = 1.f;

    float MinLifeTime = 0;
    float MaxLifeTime = 1;

    float Radius = 0.5f;

    REFLECT_TOP(FEmitterData
        , PROPERTY(MinNum)
        , PROPERTY(MaxNum)
        , PROPERTY(MinScale)
        , PROPERTY(MaxScale)
        , PROPERTY(MinLifeTime)
        , PROPERTY(MaxLifeTime)
        , PROPERTY(Radius)
    )
};

struct FParticle
{
    Vec3 Position = VEC3ZERO;
    Vec3 Velocity = VEC3ZERO;
    Vec3 Scale = VEC3ONE;
    float LifeTime = 1.f;
};

class ENGINE_DLL MFXComponent : public MPrimitiveComponent
{
public:
    MFXComponent();

public:
    virtual void BeginPlay() override;
    virtual const bool GetPrimitiveData(std::vector<FPrimitiveData>& PrimitiveDataList) override;

public:
    void ActivateFX();
    std::vector<FParticle> Particles;

public:
    std::shared_ptr<MMesh> GetMesh();
protected:
    bool bAutoActivate = true;
    FEmitterData EmitterData;
    MStructuredBuffer StructuredBuffer;
    std::shared_ptr<MMaterial> Material;
    std::shared_ptr<MMesh> Mesh;

    REFLECT(MFXComponent
        , PROPERTY(EmitterData)
        , PROPERTY(Material)
        , PROPERTY(Mesh)
    )
};

/*
std::vector<FParticle> ParticleSpawnData;
uint32 ParticleNum = random(MinNum, MaxNum);
for (uint i = 0; i < ParticleNum; ++i)
{
    FParticle NewParticle = {};
    NewParticle.Position = MakeRandomPos();
    NewParticle.Velocity = MakeRandomVelocity();
    NewParticle.LifeTime = random(InSpawnData.MinLifeTime, InSpawnData.MaxLifeTime);

    ParticleSpawnData.push_back(NewParticle);
}

// 그래픽 디바이스 함수 쓰기
uint32 StructSize = static_cast<uint32>(sizeof(FParticle));
std::shared_ptr<MVertexBuffer> StructuredBuffer = std::make_shared<MVertexBuffer>(StructSize, ParticleNum, ParticleSpawnData.data());

// 그래픽 디바이스 함수 쓰기
std::vector<Vertex> Vertices(ParticleNum);
uint32 VertexSize = static_cast<uint32>(sizeof(Vertex));
std::shared_ptr<MVertexBuffer> VertexBuffer = std::make_shared<MVertexBuffer>(VertexSize, ParticleNum, Vertices.data());

// 렌더러 쪽에서 호출하기
getGraphicDevice()->DrawInstance(VertexBuffer, nullptr, StructuredBuffer);
*/
