#include "BillboardComponent.h"

#include "Render.h"
#include "MainGame.h"
#include "Camera.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Core/ResourceManager.h"
#include "Material.h"
#include "Texture.h"

using namespace DirectX;

MBillboardComponent::MBillboardComponent()
{
    Mesh = std::make_shared<StaticMesh>();
    Mesh->LoadFromAsset(TEXT("Base/Plane.json"));

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
    XMVECTOR Look = XMVector3Normalize(XMLoadFloat3(&getWorldTranslation()) - XMLoadFloat3(&g_pMainGame->getMainCamera()->GetWorldTranslation()));
    XMVECTOR Right = XMVector3Cross(XMLoadFloat3(&VEC3UP), Look);
    XMVECTOR Up = XMVector3Cross(Look, Right);

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
        NewPrimitiveData.PrimitiveComponent = shared_from_this();
        NewPrimitiveData.MeshData = Mesh->GetMeshData(0);
        NewPrimitiveData.Material = Mesh->getGeometryLinkMaterialIndex().size() > 0 ? Mesh->getMaterials()[Mesh->getGeometryLinkMaterialIndex()[0]] : Mesh->getMaterials()[0];
        NewPrimitiveData.PrimitiveType = EPrimitiveType::Mesh;

        PrimitiveDataList.push_back(NewPrimitiveData);

        return true;
    }

    return false;
}

