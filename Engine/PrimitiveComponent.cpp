#include "Include.h"
#include "PrimitiveComponent.h"

#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"
#include "Material.h"

#include "Renderer.h"

#include "MainGame.h"
#include "MainGameSetting.h"
#include "Camera.h"

// FMeshData 참조용
#include "Mesh/Mesh.h"

using namespace DirectX;

MBoundingBox::MBoundingBox(const Vec3 &min, const Vec3 &max)
	: _min(min)
	, _max(max)
{
	// 정면
	_vertices.push_back({ Vec4{ _min.x, _min.y, _min.z, 1.f } });
	_vertices.push_back({ Vec4{ _min.x, _max.y, _min.z, 1.f } });
	_vertices.push_back({ Vec4{ _max.x, _max.y, _min.z, 1.f } });
	_vertices.push_back({ Vec4{ _min.x, _min.y, _min.z, 1.f } });
	_vertices.push_back({ Vec4{ _max.x, _max.y, _min.z, 1.f } });
	_vertices.push_back({ Vec4{ _max.x, _min.y, _min.z, 1.f } });

	// 왼쪽면
    _vertices.push_back({ Vec4{ _min.x, _min.y, _min.z, 1.f } });
    _vertices.push_back({ Vec4{ _min.x, _min.y, _max.z, 1.f } });
    _vertices.push_back({ Vec4{ _min.x, _max.y, _min.z, 1.f } });
    _vertices.push_back({ Vec4{ _min.x, _max.y, _max.z, 1.f } });
    _vertices.push_back({ Vec4{ _min.x, _min.y, _min.z, 1.f } });
    _vertices.push_back({ Vec4{ _min.x, _min.y, _max.z, 1.f } });

	// 아랫면
    _vertices.push_back({ Vec4{ _min.x, _min.y, _min.z, 1.f } });
    _vertices.push_back({ Vec4{ _min.x, _min.y, _max.z, 1.f } });
    _vertices.push_back({ Vec4{ _max.x, _min.y, _max.z, 1.f } });
    _vertices.push_back({ Vec4{ _min.x, _min.y, _min.z, 1.f } });
    _vertices.push_back({ Vec4{ _max.x, _min.y, _max.z, 1.f } });
    _vertices.push_back({ Vec4{ _max.x, _min.y, _min.z, 1.f } });

	// 오른쪽면
    _vertices.push_back({ Vec4{ _max.x, _max.y, _max.z, 1.f } });
    _vertices.push_back({ Vec4{ _max.x, _min.y, _max.z, 1.f } });
    _vertices.push_back({ Vec4{ _max.x, _min.y, _min.z, 1.f } });
    _vertices.push_back({ Vec4{ _max.x, _max.y, _max.z, 1.f } });
    _vertices.push_back({ Vec4{ _max.x, _min.y, _min.z, 1.f } });
    _vertices.push_back({ Vec4{ _max.x, _max.y, _min.z, 1.f } });

	// 윗면
	_vertices.push_back({ Vec4{ _max.x, _max.y, _max.z, 1.f } });
	_vertices.push_back({ Vec4{ _max.x, _max.y, _min.z, 1.f } });
	_vertices.push_back({ Vec4{ _min.x, _max.y, _min.z, 1.f } });
	_vertices.push_back({ Vec4{ _max.x, _max.y, _max.z, 1.f } });
	_vertices.push_back({ Vec4{ _min.x, _max.y, _min.z, 1.f } });
	_vertices.push_back({ Vec4{ _min.x, _max.y, _max.z, 1.f } });

	// 뒷면
	_vertices.push_back({ Vec4{ _max.x, _max.y, _max.z, 1.f } });
	_vertices.push_back({ Vec4{ _min.x, _max.y, _max.z, 1.f } });
	_vertices.push_back({ Vec4{ _max.x, _min.y, _max.z, 1.f } });
	_vertices.push_back({ Vec4{ _max.x, _min.y, _max.z, 1.f } });
	_vertices.push_back({ Vec4{ _min.x, _max.y, _max.z, 1.f } });
	_vertices.push_back({ Vec4{ _min.x, _min.y, _max.z, 1.f } });

	//_pVertexBuffer = std::make_shared<MVertexBuffer>(CastValue<uint32>(sizeof(Vertex)), CastValue<uint32>(_vertices.size()), _vertices.data());

	_pMaterial = std::make_shared<MMaterial>();
	_pMaterial->setShader(TEXT("VertexShader.cso"), TEXT("PixelShader.cso"));
	_pMaterial->setFillMode(Graphic::FillMode::WireFrame);
	_pMaterial->setCullMode(Graphic::CullMode::None);

    std::shared_ptr<FMeshData> MeshData = std::make_shared<FMeshData>();
    MeshData->Vertices = _vertices;
    MeshData->Indices = _indices;

    MeshDatas.push_back(MeshData);
}

std::shared_ptr<MVertexBuffer> MBoundingBox::getVertexBuffer()
{
	return _pVertexBuffer;
}

std::shared_ptr<MMaterial> MBoundingBox::getMaterial()
{
	return _pMaterial;
}

std::shared_ptr<FMeshData> MBoundingBox::GetMeshData() const
{
    return MeshDatas[0];
}

const bool MBoundingBox::cull(const std::vector<XMVECTOR> palnes, const Vec3 &position)
{
	for (auto &palne : palnes)
	{
		Vec3 dot = VEC3ZERO;
		XMStoreFloat3(&dot, XMPlaneDotCoord(palne, XMVectorSet(position.x, position.y, position.z, 1.f)));
		if (dot.x < 0.f)
		{
			return false;
		}
	}

	return true;
}

const bool MBoundingBox::cullSphere(const std::vector<XMVECTOR> palnes, const Vec3 &position, const float radius)
{
	for (auto &palne : palnes)
	{
		Vec3 dot = VEC3ZERO;
		XMStoreFloat3(&dot, XMPlaneDotCoord(palne, XMVectorSet(position.x, position.y, position.z, 1.f)));
		if (dot.x < -radius)
		{
			return false;
		}
	}

	return true;
}

const float MBoundingBox::GetLength(const Vec3 &scale /*= { 1.f, 1.f, 1.f }*/) const
{
	XMFLOAT3 length;
	XMStoreFloat3(&length, XMVector3Length(XMLoadFloat3(&scale) * (XMLoadFloat3(&_max) - XMLoadFloat3(&_min))));

	return length.x;
}

uint32 MPrimitiveComponent::PrimitiveCounter = 0;

MPrimitiveComponent::MPrimitiveComponent()
	:_eRenderMdoe{ RenderMode::Perspective }
	, PrimitiveID{ PrimitiveCounter++ }
{
}

MPrimitiveComponent::~MPrimitiveComponent()
{
}

void MPrimitiveComponent::Update(const Time deltaTime)
{
	SceneComponent::Update(deltaTime);

	g_pRenderer->AddPrimitive(shared_from_this());
}

const bool MPrimitiveComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	return false;
}

const bool MPrimitiveComponent::GetBoundingBox(std::shared_ptr<MBoundingBox> &boundingBox)
{
	return false;
}

void MPrimitiveComponent::setRenderMode(const RenderMode renderMode)
{
	_eRenderMdoe = renderMode;
}

const MPrimitiveComponent::RenderMode MPrimitiveComponent::getRenderMdoe() const
{
	return _eRenderMdoe;
}

void MPrimitiveComponent::SetRendering(bool bNewRendering)
{
	if (bNewRendering == bRendering)
	{
		return;
	}

	bRendering = bNewRendering;
}

void MPrimitiveComponent::setDrawingBoundingBox(const bool bDraw)
{
    _bDrawBoundingBox = bDraw;
}

const bool MPrimitiveComponent::IsDrawingBoundingBox() const
{
    return _bDrawBoundingBox;
}
