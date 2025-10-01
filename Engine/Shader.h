#pragma once

#include "ConstantBuffer.h"

// 쉐이더는 hlsl에서 사용되는 변수 값과, CBuffer를 관리를 하는 존재임
// 지금은 쉐이더마다 CBuffer를 생성하고 있지만, 이후에는 공통된 CBuffer를 공유하는 구조로 변경하고 싶음
enum class EConstantBufferLayer
{
	Global,		// 해상도 등 설정 값
	Tick,		// 카메라 위치? 등등9
	Object,		// World, View, Proj 등등
	Count
};

enum class ShaderType : uint8
{
	Vertex, 
	Pixel, 
	Geometry,
	Count
};

struct FBufferVariable
{
public:
	FBufferVariable(uint32 offset, uint32 size)
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
	FBufferVariable(const FBufferVariable &rhs)
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

	~FBufferVariable()
	{
		if (Value != nullptr)
		{
			delete[] Value;
			Value = nullptr;
			Size = -1;
			Offset = -1;
		}
	}

	FBufferVariable& operator=(FBufferVariable &rhs)
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
    uint32 Offset = 0;
	uint32 Size = 0;
	Byte* Value = nullptr;
};

struct FBufferVariableInfo
{
public:
	FBufferVariableInfo()
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
    // ConstantBuffer에 저장된 데이터를 GPU에 전달함 
    void Apply();
    virtual void SetToDevice();
protected:
    std::vector<ID3D11Buffer*> GetBuffers();

public:
	ID3D10Blob* getBlob();
private:
	ID3D10Blob *_pBlob = nullptr;

/*----------------------------------------------------------------------
Shader는 자체적으로 hlsl에서 사용되는 변수들의 값을 관리하는데, ConstantBuffer의 데이터와는 별개의 존재임.
SetValue 함수로 값을 설정할 수 있고, 버퍼에 올리기 위해서는 Commit을 해야함.
----------------------------------------------------------------------*/
public:
    // 일반 자료형 대응
	template <class T>
	void SetValue(const std::wstring& InName, const T& InValue)
	{
        GetConstantBuffer()->SetData(InName, static_cast<const void*>(&InValue));
	}
    // 포인터 타입 대응
	template <class T>
	void SetValue(const std::wstring& InName, T* InValue)
	{
        GetConstantBuffer()->SetData(InName, static_cast<const void*>(InValue));
    }
    // 벡터 타입 대응
	template <class T>
	void SetValue(const std::wstring& InName, const std::vector<T>& InValue)
	{
        GetConstantBuffer()->SetData(InName, static_cast<const void*>(InValue.data()));
	}

	void UpdateConstantBuffer(const EConstantBufferLayer layer, std::vector<FBufferVariable> &varialbeInfos);
	void UpdateConstantBuffer(const EConstantBufferLayer layer);

private:
	void CreateCosntantBuffers();
    std::shared_ptr<MConstantBuffer> ParsingBuffer(ID3D11ShaderReflection* InShaderReflection,  EConstantBufferLayer InLayer, uint32& BufferrIndexer);

public:
	const uint32 getVariableCountOfConstantBuffer(const EConstantBufferLayer layer);
    std::shared_ptr<MConstantBuffer> GetConstantBuffer();
    bool HasConstantBuffer() { return GetConstantBuffer() != nullptr; }
protected:
	// ObjectLayer 버퍼만 관리
	std::vector<std::shared_ptr<MConstantBuffer>> ConstantBuffers;

public:
	std::vector<std::vector<FBufferVariable>>& GetVariables();
private:
	// ConstantBuffer 레이어 별로 변수 정보 저장
	std::vector<std::vector<FBufferVariable>> Variables;	
	// 이름과 변수의 바인딩 정보
	std::unordered_map<std::wstring, FBufferVariableInfo> VariableInfos;

public:
    static std::shared_ptr<MConstantBuffer>& GetSharedConstantBuffer(EConstantBufferLayer InLayer);
protected:
    // 모든 쉐이더가 공유하는 버퍼, 현재는 Global, Tick 이 해당됨.
    static std::vector<std::shared_ptr<MConstantBuffer>> SharedBuffers;
};