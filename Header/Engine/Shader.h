#pragma once
#ifndef __SHADER_H__
#define __SHADER_H__

class ConstantBuffer;

enum class ConstantBuffersLayer
{
	Constant,		// �ػ� �� ���� ��
	PerTick,		// ī�޶� ��ġ? ���
	PerObject,		// World, View, Proj ���
	Count
};

enum class ShaderType : uint8
{
	Vertex, 
	Pixel, 
	Count
};


struct VariableInfo
{
public:
	VariableInfo(uint32 offset, uint32 size)
		: _offset{ offset }
		, _size{ size }
		, _pValue{ nullptr }
	{
		if (_size > 0)
		{
			_pValue = new Byte[_size];
			ZeroMemory(_pValue, _size);
		}
	}
	VariableInfo(const VariableInfo &rhs)
	{
		_offset = rhs._offset;
		_size = rhs._size;

		if (_size > 0)
		{
			_pValue = new Byte[_size];
			memcpy(_pValue, rhs._pValue, _size);
		}
	}

	~VariableInfo()
	{
		if (_pValue != nullptr)
		{
			delete[] _pValue;
		}
	}

public:
	uint32 _offset;
	uint32 _size;
	Byte* _pValue;
};

class Shader abstract : public std::enable_shared_from_this<Shader>
{
public:
	explicit Shader(const std::wstring &filePathName);
	~Shader();

	// d3d11 raw
public:
	virtual void SetToDevice() = 0;

public:
	ID3D10Blob* getBlob();
private:
	ID3D10Blob *_pBlob = nullptr;

	// cbuffer
public:
	void UpdateConstantBuffer(const ConstantBuffersLayer layer, std::vector<VariableInfo> &varialbeInfos);

private:
	void MakeCosntantBuffers();

public:
	const uint32 getVariableCountOfConstantBuffer(const ConstantBuffersLayer layer);
protected:
	std::vector<std::shared_ptr<ConstantBuffer>> _constantBuffers;	// �ӵ��� ���� ���̾�� �ϳ��� ���۸� �����ϱ�!

public:
	std::vector<std::vector<VariableInfo>>& getVariableInfo();
private:
	std::vector<std::vector<VariableInfo>> _variableInfos;
};

#endif