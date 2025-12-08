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

    g_ResourceManager->Load(TEXT("Base/Plane.json"), Mesh);
    g_ResourceManager->Load(TEXT("Resources/Texture/Player.jpeg"), Texture);
    if (Mesh->getMaterial(0))
    {
        Mesh->getMaterial(0)->setTexture(ETextureType::Diffuse, Texture);
        Mesh->getMaterial(0)->setCullMode(Graphic::CullMode::None);
    }

    bShadowing = false;
}

Mat4& MBillboardComponent::getWorldMatrix()
{
    auto& World = GetOwner()->GetOwner()->CastToShared<MWorld>();

    XMVECTOR Look = XMVector3Normalize(XMLoadFloat3(&getWorldTranslation()) - XMLoadFloat3(&World->getMainCamera()->GetWorldTranslation()));
    XMVECTOR Right = XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&VEC3UP), Look));
    XMVECTOR Up = XMVector3Normalize(XMVector3Cross(Look, Right));

    XMMATRIX Mat = XMLoadFloat4x4(&ZEROMATRIX);
    Mat.r[0] = Right;
    Mat.r[1] = Up;
    Mat.r[2] = Look;
    Mat.r[3] = XMVectorSet(getTranslation().x, getTranslation().y, getTranslation().z, 1.f);
    
    XMStoreFloat4x4(&BillboardWorldMat, Mat);

    return BillboardWorldMat;
}

const bool MBillboardComponent::GetPrimitiveData(std::vector<FPrimitiveData>& PrimitiveDataList)
{
    if (Mesh)
    {
        FPrimitiveData NewPrimitiveData = {};
        NewPrimitiveData.PrimitiveComponent = GetShared();
        NewPrimitiveData.MeshData = &Mesh->GetMeshData(0);
        NewPrimitiveData.Material = Mesh->getGeometryLinkMaterialIndex().size() > 0 ? Mesh->getMaterials()[Mesh->getGeometryLinkMaterialIndex()[0]] : Mesh->getMaterials()[0];
        NewPrimitiveData.PrimitiveType = EPrimitiveType::Mesh;

        PrimitiveDataList.push_back(NewPrimitiveData);

        return true;
    }

    return false;
}

