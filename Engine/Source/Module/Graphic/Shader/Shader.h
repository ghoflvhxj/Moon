#pragma once

#include "Include.h"
#include "ConstantBuffer.h"
#include "Module/Graphic/StructuredBuffer.h"

// Common.hlsli의 설명을 참조
enum class EConstantBufferLayer
{
	Global,
	Tick,
    RenderPass,
    RenderPassObject,
    Material,
	Object,
    Custom,

	Count
};

enum class EStructuredBufferLayer
{
    Layer0,
    Count
};

enum class EShaderType : uint8
{
	Vertex, 
	Pixel, 
	Geometry,
    Compute,
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
	virtual ~MShader();

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

public:
    bool IsType(EShaderType InType) { return InType == ShaderType; }
    bool IsVertexShader() { return IsType(EShaderType::Vertex); }
    bool IsPixelShader() { return IsType(EShaderType::Pixel); }
protected:
    EShaderType ShaderType = EShaderType::Count;

    /***********************************************************************
     Shader는 ConstantBuffer를 관리하며, 변수들에 대한 정보를 가지고 있음.
     SetValue 함수로 ConstantBuffer에 값을 설정할 수 있고, 버퍼에 올리기 위해서는 Commit을 해야함.
    ***********************************************************************/
public:
    // 일반 자료형 대응
	template <class T>
	void SetValue(const std::wstring& InName, const T& InValue)
	{
        auto& Iter = VariableInfos.find(InName);
        if (Iter != VariableInfos.end())
        {
            GetConstantBuffer(Iter->second.Layer)->SetData(InName, static_cast<const void*>(&InValue));
        }
	}

    // bool 특수화
    template <>
    void SetValue(const std::wstring& InName, const bool& InValue)
    {
        const auto& Iter = VariableInfos.find(InName);
        if (Iter != VariableInfos.end())
        {
            BOOL BigBool = InValue ? TRUE : FALSE;
            GetConstantBuffer(Iter->second.Layer)->SetData(InName, &BigBool);
        }
    }

    // 포인터 타입 대응
	template <class T>
	void SetValue(const std::wstring& InName, T* InValue)
	{
        auto& Iter = VariableInfos.find(InName);
        if (Iter != VariableInfos.end())
        {
            GetConstantBuffer(Iter->second.Layer)->SetData(InName, static_cast<const void*>(InValue));
        }
    }
    // 벡터 타입 대응
	template <class T>
	void SetValue(const std::wstring& InName, const std::vector<T>& InValue)
	{
        auto& Iter = VariableInfos.find(InName);
        if (Iter != VariableInfos.end())
        {
            GetConstantBuffer(Iter->second.Layer)->SetData(InName, static_cast<const void*>(InValue.data()));
        }
	}

    /*********************************************
        CBuffer 관련 기능들
    *********************************************/
private:
	void ParseShader();
    std::shared_ptr<MConstantBuffer> ParseBuffer(ID3D11ShaderReflection* InShaderReflection, uint32 InBufferIndex, EConstantBufferLayer& OutLayer);

public:
	const uint32 getVariableCountOfConstantBuffer(const EConstantBufferLayer layer);
    std::shared_ptr<MConstantBuffer> GetConstantBuffer(EConstantBufferLayer InLayer);
    bool HasConstantBuffer(EConstantBufferLayer InLayer);
protected:
	// 전역이 아닌 CosntantBuffer를 관리함
	std::vector<std::shared_ptr<MConstantBuffer>> ConstantBuffers;

public:
    static std::shared_ptr<MConstantBuffer>& GetSharedConstantBuffer(EConstantBufferLayer InLayer);
    static std::vector<std::shared_ptr<MConstantBuffer>>& GetSharedConstantBuffers() { return SharedBuffers; }
protected:
    // 모든 쉐이더가 공유하는 버퍼, 현재는 Global, Tick 이 해당됨.
    static std::vector<std::shared_ptr<MConstantBuffer>> SharedBuffers;

public:
    MStructuredBuffer StructuredBuffer;
    MStructuredBuffer RWStructuredBuffer;

private:
	// 모든 ConstantBuffer 변수의 이름과 변수의 바인딩 정보 저장
	std::unordered_map<std::wstring, FBufferVariableInfo> VariableInfos;
};

class ENGINE_DLL MNewShader
{
public:
    MNewShader() = default;
    virtual ~MNewShader() = default;

public:
    bool IsValid() const;

public:
    void SetShaderID(uint32 InID);
    uint32 GetShaderID() const;
protected:
    int32 ShaderID = -1;
};