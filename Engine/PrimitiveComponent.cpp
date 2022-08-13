#include "stdafx.h"
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
	// ����
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });

	// ���ʸ�
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });

	// �Ʒ���
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });

	// �����ʸ�
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });

	// ����
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });

	// �޸�
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });

	_pVertexBuffer = std::make_shared<VertexBuffer>(CastValue<uint32>(sizeof(Vertex)), CastValue<uint32>(_vertices.size()), _vertices.data());

	_pMaterial = std::make_shared<Material>();
	_pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso")); // ������ ������ ���̴��� �о�� �ϴµ�, ������ �����ϱ� �׳� �ӽ÷� ����
	_pMaterial->setFillMode(Graphic::FillMode::WireFrame);
}

std::shared_ptr<VertexBuffer> BoundingBox::getVertexBuffer()
{
	return _pVertexBuffer;
}

std::shared_ptr<Material> BoundingBox::getMaterial()
{
	return _pMaterial;
}


const bool BoundingBox::Cull(const std::vector<XMVECTOR> palnes, const Vec3 &position)
{
	for (auto &palne : palnes)
	{
		Vec3 dot = VEC3ZERO;
		XMStoreFloat3(&dot, XMPlaneDot(palne, XMVectorSet(position.x, position.y, position.z, 1.f)));
		if (dot.x < 0.f)
		{
			return false;
		}
	}

	return true;
}


PrimitiveComponent::PrimitiveComponent()
	:_eRenderMdoe{ RenderMode::Perspective }
{
}

PrimitiveComponent::~PrimitiveComponent()
{
}

void PrimitiveComponent::Update(const Time deltaTime)
{
	SceneComponent::Update(deltaTime);

	g_pRenderer->addPrimitiveComponent(shared_from_this());
}

const bool PrimitiveComponent::getPrimitiveData(std::vector<PrimitiveData> &primitiveDataList)
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
