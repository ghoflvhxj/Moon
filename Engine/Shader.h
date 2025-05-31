#pragma once
#ifndef __SHADER_H__
#define __SHADER_H__

#include "ConstantBuffer.h"

enum class EConstantBufferLayer
{
	Constant,		// 해상도 등 설정 값
	PerTick,		// 카메라 위치? 등등
	PerObject,		// World, View, Proj 등등
	Count
};

enum class ShaderType : uint8
{
	Vertex, 
	Pixel, 
	Geometry,
	Count
};


struct FShaderVariable
{
public:
	FShaderVariable(uint32 offset, uint32 size)
		: Offset{ offset }
		, Size{ size }
		, Value{ nullptr }
	{
		if (Size > 0)
		{
			Value = new Byte[Size];
			ZeroMemory(Value, Size);
		}
	}
	FShaderVariable(const FShaderVariable &rhs)
	{
		Offset = rhs.Offset;
		Size = rhs.Size;
		Value = nullptr;

		if (Size > 0)
		{
			Value = new Byte[Size];
			memcpy(Value, rhs.Value, Size);
		}
	}

	~FShaderVariable()
	{
		if (Value != nullptr)
		{
			delete[] Value;
			Value = nullptr;
			Size = -1;
			Offset = -1;
		}
	}

	FShaderVariable& operator=(FShaderVariable &rhs)
	{
		if (Value != nullptr)
		{
			delete[] Value;
			Value = nullptr;
			Size = -1;
			Offset = -1;
		}

		Offset = rhs.Offset;
		Size = rhs.Size;
		Value = nullptr;

		if (Size > 0)
		{
			Value = new Byte[Size];
			memcpy(Value, rhs.Value, Size);
		}

		return *this;
	}

public:
	uint32 Offset;
	uint32 Size;
	Byte* Value;
};

struct FShaderVariableInfo
{
public:
	FShaderVariableInfo()
		: Layer(EConstantBufferLayer::Count), Index(-1)
	{
	}
public:
	EConstantBufferLayer Layer;
	int32 Index;
};

class MShader : public std::enable_shared_from_this<MShader>
{
public:
	explicit MShader(const std::wstring &filePathName);
	~MShader();

	// d3d11 raw
public:
	virtual void SetToDevice() = 0;

public:
	ID3D10Blob* getBlob();
private:
	ID3D10Blob *_pBlob = nullptr;

	// cbuffer
public:
	template <class T>
	void SetValue(const std::wstring& InName, const T& InValue)
	{
		if(VariableInfos.find(InName) == VariableInfos.end())
		{
			return;
		}

		const FShaderVariableInfo& ShaderVariableInfo = VariableInfos[InName];
		const std::vector<FShaderVariable>& LayerVariables = Variables[static_cast<int32>(ShaderVariableInfo.Layer)];
		if (ShaderVariableInfo.Index >= LayerVariables.size())
		{
			return;
		}

		const FShaderVariable& ShaderVariable = LayerVariables[ShaderVariableInfo.Index];

		memcpy(ShaderVariable.Value, &InValue, ShaderVariable.Size);
		ConstantBuffers[static_cast<int32>(ShaderVariableInfo.Layer)]->SetData(ShaderVariable.Offset, ShaderVariable.Value, ShaderVariable.Size);
	}
	template <class T>
	void SetValue(const std::wstring& InName, const std::vector<T>& InValue)
	{
		if (VariableInfos.find(InName) == VariableInfos.end())
		{
			return;
		}

		const FShaderVariableInfo& ShaderVariableInfo = VariableInfos[InName];
		const std::vector<FShaderVariable>& LayerVariables = Variables[static_cast<int32>(ShaderVariableInfo.Layer)];
		if (ShaderVariableInfo.Index >= LayerVariables.size())
		{
			return;
		}

		const FShaderVariable& ShaderVariable = LayerVariables[ShaderVariableInfo.Index];

		memcpy(ShaderVariable.Value, InValue.data(), ShaderVariable.Size);
		ConstantBuffers[static_cast<int32>(ShaderVariableInfo.Layer)]->SetData(ShaderVariable.Offset, ShaderVariable.Value, ShaderVariable.Size);
	}

	void UpdateConstantBuffer(const EConstantBufferLayer layer, std::vector<FShaderVariable> &varialbeInfos);
	void UpdateConstantBuffer(const EConstantBufferLayer layer);

private:
	void CreateCosntantBuffers();

public:
	const uint32 getVariableCountOfConstantBuffer(const EConstantBufferLayer layer);
protected:
	// 레이어 별로 ConstantBuffer를 관리
	std::vector<std::shared_ptr<MConstantBuffer>> ConstantBuffers;

public:
	std::vector<std::vector<FShaderVariable>>& GetVariableInfos();
private:
	// ConstantBuffer 레이어 별로 변수 정보 저장
	std::vector<std::vector<FShaderVariable>> Variables;	
	// 이름과 변수의 바인딩 정보
	std::unordered_map<std::wstring, FShaderVariableInfo> VariableInfos;
	// ConstantBuffer 를 업데이트 하는데 사용되는 데이터
	//std::vector<Byte*> Buffers;
};

#endif