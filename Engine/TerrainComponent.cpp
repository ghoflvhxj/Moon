#include "Include.h"
#include "TerrainComponent.h"

#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Material.h"

#include "Texture.h"

#include "MainGame.h"
#include "Camera.h"

using namespace DirectX;

TerrainComponent::TerrainComponent()
	: _tileNumX{ 10 }, _tileNumY{ 10 }, _interval{ 1.f }
{
	initializeMeshInfromation();
}

TerrainComponent::TerrainComponent(const TileNum x, const TileNum y)
	: _tileNumX{ x }, _tileNumY{ y }, _interval{ 1.f }
{
	initializeMeshInfromation();
}

TerrainComponent::TerrainComponent(const TileNum x, const TileNum y, const TileInterval interval)
	: _tileNumX{ x }, _tileNumY{ y }, _interval{ interval }
{
	initializeMeshInfromation();
}

TerrainComponent::~TerrainComponent()
{
}

void TerrainComponent::initializeMeshInfromation()
{
	_textureList.resize(5, nullptr);

	Vec4 color = static_cast<Vec4>(EngineColors::White);

	uint32 vertexNumX = _tileNumX + 1;
	uint32 vertexNumY = _tileNumY + 1;
	uint32 triNum = 2 * (vertexNumX - 1) * (vertexNumY - 1);

	// ���ؽ�
	_vertexList.reserve(vertexNumX * vertexNumY);
	for (uint32 i = 0; i < vertexNumY; ++i)
	{
		for (uint32 j = 0; j < vertexNumX; ++j)
		{
			Vec3 pos = { j * _interval, 0.f , i * _interval };
			Vec2 uv = { 1.f * j, 1.f * (_tileNumY - i) };
			Vec3 normal = { 0.f, 1.f, 0.f };
			Vec3 tangent = { 1.f, 0.f, 0.f };
			Vec3 binormal = { 0.f, 0.f, 1.f };
			_vertexList.push_back(Vertex{ pos, color, uv, normal, tangent, binormal });
		}
	}

	// �ε���
	_indexList.reserve(3 * triNum);
	for (TileNum i = 0; i < _tileNumY; ++i)
	{
		for (TileNum j = 0; j < _tileNumX; ++j)
		{
			Index base = (vertexNumX * i) + j;

			_indexList.push_back(base);
			_indexList.push_back(base + vertexNumX);
			_indexList.push_back(base + 1);

			_indexList.push_back(base + 1);
			_indexList.push_back(base + vertexNumX);
			_indexList.push_back(base + vertexNumX + 1);
		}
	}

	_pMaterial = std::make_shared<MMaterial>();
	_pMaterial->setTextures(_textureList);
	_pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso")); // ������ ������ ���̴��� �о�� �ϴµ�, ������ �����ϱ� �׳� �ӽ÷� ����
}

//void TerrainComponent::render()
//{ 
//	_pMaterial->render(shared_from_this());
//}

const TerrainComponent::TileNum TerrainComponent::getTileX() const
{
	return _tileNumX;
}

const TerrainComponent::TileNum TerrainComponent::getTileY() const
{
	return _tileNumY;
}

const TerrainComponent::TileInterval TerrainComponent::getTileInterval() const
{
	return _interval;
}

const bool TerrainComponent::Test(const Vec3 &pos, float *pY)
{
	// ���� ��ġ�� Ÿ�� ���ϱ�
	const TileNum tileX = static_cast<TileNum>(pos.x / _interval);
	const TileNum tileY = static_cast<TileNum>(pos.z / _interval);
	const uint32 index = tileX + (tileY * _tileNumX);

	// �ش� Ÿ���� ���ؽ� �ε��� ���ϱ�
	Index base = index * 6;

	// Ÿ���� ��� �ﰢ���� �ִ��� ���ϱ�
	float xzRatio = pos.x - (int)pos.x / pos.z - (int)pos.z;
	bool isRight = false;
	if (xzRatio <= 1.f)
	{
		isRight = true;
		base += 3;
	}

	// ����� ������ �̿��ؼ� y�� ���ϱ�
	XMVECTOR points[3] = {
		XMLoadFloat3(&_vertexList[_indexList[base + 0]].Pos),
		XMLoadFloat3(&_vertexList[_indexList[base + 1]].Pos),
		XMLoadFloat3(&_vertexList[_indexList[base + 2]].Pos),
	};
	XMVECTOR plane = XMPlaneFromPoints(points[0], points[1], points[2]);
	Vec4 p;
	XMStoreFloat4(&p, plane);

	*pY = ((pos.x * p.x) + (pos.z * p.z) + p.w) / -p.y;

	return true;
}

const bool TerrainComponent::addTexture(std::shared_ptr<MTexture> pTexture)
{
	if (_textureList.size() == _textureList.capacity())
		return false;

	_textureList.push_back(pTexture);
	return true;
}

void TerrainComponent::setTexture(const uint32 index, std::shared_ptr<MTexture> pTexture)
{
	_textureList[index] = pTexture;

	// ���͸��� ���ο� �ؽ��ĸ� ���ε� ����
	_pMaterial->setTextures(_textureList);
}

void TerrainComponent::setMaterial(std::shared_ptr<MMaterial> pMaterial)
{
	_pMaterial = pMaterial;
}

std::shared_ptr<MMaterial>& TerrainComponent::getMaterial()
{
	return _pMaterial;
}