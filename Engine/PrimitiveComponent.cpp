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

using namespace DirectX;

BoundingBox::BoundingBox(const Vec3 &min, const Vec3 &max)
	: _min(min)
	, _max(max)
{
	// 정면
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });

	// 왼쪽면
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });

	// 아랫면
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });

	// 오른쪽면
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });

	// 윗면
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });

	// 뒷면
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });

	_pVertexBuffer = std::make_shared<VertexBuffer>(CastValue<uint32>(sizeof(Vertex)), CastValue<uint32>(_vertices.size()), _vertices.data());

	_pMaterial = std::make_shared<Material>();
	_pMaterial->setShader(TEXT("VertexShader.cso"), TEXT("PixelShader.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵
	_pMaterial->setFillMode(Graphic::FillMode::WireFrame);
	_pMaterial->setCullMode(Graphic::CullMode::None);
}

std::shared_ptr<VertexBuffer> BoundingBox::getVertexBuffer()
{
	return _pVertexBuffer;
}

std::shared_ptr<Material> BoundingBox::getMaterial()
{
	return _pMaterial;
}


const bool BoundingBox::cull(const std::vector<XMVECTOR> palnes, const Vec3 &position)
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

const bool BoundingBox::cullSphere(const std::vector<XMVECTOR> palnes, const Vec3 &position, const float radius)
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

const float BoundingBox::getLength(const Vec3 &scale /*= { 1.f, 1.f, 1.f }*/) const
{
	XMFLOAT3 length;
	XMStoreFloat3(&length, XMVector3Length(XMLoadFloat3(&scale) * XMVectorSet(_max.x - _min.x, _max.x - _min.x, _max.x - _min.x, 0.f)));

	return length.x;
}

uint32 PrimitiveComponent::PrimitiveCounter = 0;

PrimitiveComponent::PrimitiveComponent()
	:_eRenderMdoe{ RenderMode::Perspective }
	, PrimitiveID{ PrimitiveCounter++ }
{
}

PrimitiveComponent::~PrimitiveComponent()
{
}

void PrimitiveComponent::Update(const Time deltaTime)
{
	SceneComponent::Update(deltaTime);

	g_pRenderer->AddPrimitive(shared_from_this());
}

const bool PrimitiveComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	return false;
}

const bool PrimitiveComponent::getBoundingBox(std::shared_ptr<BoundingBox> &boundingBox)
{
	return false;
}

void PrimitiveComponent::setRenderMode(const RenderMode renderMode)
{
	_eRenderMdoe = renderMode;
}

const PrimitiveComponent::RenderMode PrimitiveComponent::getRenderMdoe() const
{
	return _eRenderMdoe;
}

void PrimitiveComponent::SetRendering(bool bNewRendering)
{
	if (bNewRendering == bRendering)
	{
		return;
	}

	bRendering = bNewRendering;
}
