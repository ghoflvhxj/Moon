#include "BillboardComponent.h"

#include "Render.h"
#include "World.h"
#include "Camera.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Core/ResourceManager.h"

#include "Material.h"
#include "Texture.h"

using namespace DirectX;

MBillboardComponent::MBillboardComponent()
{
    //Mesh = std::make_shared<StaticMesh>();
    //Mesh->LoadFromDisk(TEXT("Base/Plane.fbx"));
    bShadowing = false;


    //g_ResourceManager->Load(TEXT("Resources/Texture/Player.jpeg"), Texture);
    //SetMesh(TEXT("Base/Plane.json"));
    Material = std::make_shared<MMaterial>();
    Material->setCullMode(Graphic::CullMode::None);
    Material->SetAlphaMask(true);
    //Material->setTexture(ETextureType::Diffuse, Texture);
}

void MBillboardComponent::Update(const Time deltaTime)
{
    MWorld* World = GetWorld();
    assert(World);

    XMVECTOR Pos = XMLoadFloat3(&getWorldTranslation());

    const Vec3& CamPos = World->getMainCamera()->getComponent(ROOT_COMPONENT)->getWorldTranslation();
    XMVECTOR XMForward = XMVector3Normalize(Pos - XMLoadFloat3(&CamPos));
    XMVECTOR XMRight = XMVector3Cross(XMLoadFloat3(&VEC3UP), XMForward);
    XMVECTOR XMUp = XMVector3Cross(XMForward, XMRight);

    XMMATRIX XMRotMat = { XMRight, XMUp, XMForward, XMVectorSet(0.f, 0.f, 0.f, 1.f) };

    Vec4 Quat = {};
    XMStoreFloat4(&Quat, XMQuaternionRotationMatrix(XMRotMat));
    Vec3 Angle = {};
    DXQuaternionToEuler(Quat, Angle.x, Angle.y, Angle.z);
    SetRotation(Angle);

    Super::Update(0.f);
}

void MBillboardComponent::OnRegisted()
{
    if (bAlwaysUpdate)
    {
        GetWorld()->AddAlwaysUpdatableComponent(GetShared());
    }

    Super::OnRegisted();
}

const bool MBillboardComponent::GetPrimitiveData(std::vector<FPrimitiveData>& PrimitiveDataList)
{
    if (Mesh)
    {
        FPrimitiveData NewPrimitiveData = {};
        NewPrimitiveData.PrimitiveComponent = GetShared();
        NewPrimitiveData.MeshData = &Mesh->GetMeshData(0);
        NewPrimitiveData.Material = Material;
        NewPrimitiveData.PrimitiveType = PrimitiveType;

        PrimitiveDataList.push_back(NewPrimitiveData);

        return true;
    }

    return false;
}

void MBillboardComponent::SetPrimitiveType(EPrimitiveType InType)
{
    PrimitiveType = InType;
}

