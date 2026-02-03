#include "FXComponent.h"
#include "MoonEngine.h"
#include "GraphicDevice.h"
#include "Render.h"

template <class T>
T RandomRange(T InMin, T InMax)
{
    std::random_device Rd;
    std::mt19937_64 Mt(Rd());

    std::conditional_t<std::is_integral_v<T>, std::uniform_int_distribution<T>, std::uniform_real_distribution<T>> Dist(InMin, InMax);
    
    return Dist(Mt);
}

template <>
Vec3 RandomRange(Vec3 InMin, Vec3 InMax)
{
    std::random_device Rd;
    std::mt19937_64 Mt(Rd());

    auto GetFlaot = [&](float InMin, float InMax) {
        std::uniform_real_distribution<float> Dist(InMin, InMax);
        return Dist(Mt);
    };

    return { GetFlaot(InMin.x, InMax.x), GetFlaot(InMin.y, InMax.y), GetFlaot(InMin.z, InMax.z) };
}

MFXComponent::MFXComponent()
{
}

void MFXComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoActivate)
    {
        ActivateFX();
    }
}

const bool MFXComponent::GetPrimitiveData(std::vector<FPrimitiveData>& PrimitiveDataList)
{
    if (Mesh)
    {
        for (const auto& MeshData : Mesh->GetMeshDatas())
        {
            FPrimitiveData NewPrimitiveData = {};
            NewPrimitiveData.PrimitiveComponent = GetShared();
            NewPrimitiveData.PrimitiveType = EPrimitiveType::FX;
            NewPrimitiveData.MeshData = &MeshData;
            NewPrimitiveData.Material = Material;

            PrimitiveDataList.push_back(NewPrimitiveData);
        }
    }

    return true;
}

void MFXComponent::ActivateFX()
{
    uint32 ParticleNum = RandomRange(EmitterData.MinNum, EmitterData.MaxNum);

    Particles.clear();
    Particles.reserve(ParticleNum);

    for (uint32 i = 0; i < ParticleNum; ++i)
    {
        FParticle NewParticle = {};

        float Radius = RandomRange(0.f, EmitterData.Radius);
        NewParticle.Position = RandomRange(Vec3{ -Radius, -Radius, -Radius }, Vec3{ Radius, Radius, Radius });

        float Scale = RandomRange(EmitterData.MinScale, EmitterData.MaxScale);
        NewParticle.Scale = Vec3{ Scale, Scale, Scale };
        NewParticle.Velocity = Vec3{ 0.f, 0.f, 0.f };
        NewParticle.LifeTime = RandomRange(EmitterData.MinLifeTime, EmitterData.MaxLifeTime);

        Particles.push_back(NewParticle);
    }

    GetPrimitiveChangedDelegate().Broadcast(this);
}

std::shared_ptr<MMesh> MFXComponent::GetMesh()
{
    return Mesh;
}


//uint32 StructSize = static_cast<uint32>(sizeof(FParticle));
//uint32 ElemNum = GetSize(ParticleSpawnData);
//StructuredBuffer = getGraphicDevice()->AddStructuredBuffer(ParticleSpawnData.data(), StructSize * ElemNum, ElemNum, StructSize, true);