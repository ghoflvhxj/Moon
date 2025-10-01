#include "Include.h"
#include "Shader.h"

#include "GraphicDevice.h"
#include "ConstantBuffer.h"

#include <memory.h>

std::vector<std::shared_ptr<MConstantBuffer>> MShader::SharedBuffers(EnumToIndex(EConstantBufferLayer::Tick) + 1, nullptr);

MShader::MShader(const std::wstring &filePathName)
	: ConstantBuffers(CastValue<uint32>(EConstantBufferLayer::Count), nullptr)
	, Variables(CastValue<uint32>(EConstantBufferLayer::Count), std::vector<FBufferVariable>())
{
	if (filePathName.empty())
	{
		return;
	}

	FAILED_CHECK_THROW(D3DReadFileToBlob(filePathName.c_str(), &_pBlob));
	CreateCosntantBuffers();
}

MShader::~MShader()
{
	SafeRelease(_pBlob);
}

void MShader::Apply()
{
    if (GetConstantBuffer())
    {
        GetConstantBuffer()->Commit();
    }

    SetToDevice();
}

void MShader::SetToDevice()
{
    // DoNothing
    // 자식 클래스에서 정의
}

std::vector<ID3D11Buffer*> MShader::GetBuffers()
{
    std::vector<ID3D11Buffer*> Buffers;
    /*
    Buffers.reserve(ConstantBuffers.size());
    for (auto& ConstantBuffer : ConstantBuffers)
    {
        Buffers.emplace_back(ConstantBuffer ? ConstantBuffer->getRaw() : nullptr);
    }
    */

    if (HasConstantBuffer())
    {
        Buffers.push_back(GetConstantBuffer()->getRaw());
    }
    else
    {
        Buffers.push_back(nullptr);
    }

    return Buffers;
}

void MShader::UpdateConstantBuffer(const EConstantBufferLayer layer, std::vector<FBufferVariable>& InVariables)
{
	uint32 Index = CastValue<uint32>(layer);
    
    for (int i=0; i< Variables[Index].size(); ++i)
    {
        Variables[Index][i] = InVariables[i];
    }

    UpdateConstantBuffer(layer);
}

void MShader::UpdateConstantBuffer(const EConstantBufferLayer layer)
{
	uint32 Index = CastValue<uint32>(layer);
	if (nullptr == ConstantBuffers[Index])
	{
		return;
	}

	for (const FBufferVariable& Variable : Variables[Index])
	{
		ConstantBuffers[Index]->SetData(Variable.Offset, Variable.Value, Variable.Size);
	}
}

void MShader::CreateCosntantBuffers()
{
	ID3D11ShaderReflection *DX_ShaderReflection = nullptr;

	FAILED_CHECK_THROW(D3DReflect(_pBlob->GetBufferPointer(), _pBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&DX_ShaderReflection));

	D3D11_SHADER_DESC DX_ShaderDesc = { 0 };
	FAILED_CHECK_THROW(DX_ShaderReflection->GetDesc(&DX_ShaderDesc));

	uint32 ConstantBufferLayerNum = CastValue<uint32>(EConstantBufferLayer::Count);
	uint32 constantBufferNum = static_cast<uint32>(DX_ShaderDesc.ConstantBuffers);
	if (constantBufferNum > ConstantBufferLayerNum)
	{
		DEV_ASSERT_MSG("ConstantBuffer의 개수가 ConstantBuffersLayer::Countf를 넘어섭니다.");
	}

    if (constantBufferNum == 0)
    {
        return;
    }

    uint32 BufferrIndexer = 0;
    if (SharedBuffers[EnumToIndex(EConstantBufferLayer::Global)] == nullptr)
    {
        SharedBuffers[EnumToIndex(EConstantBufferLayer::Global)] = ParsingBuffer(DX_ShaderReflection, EConstantBufferLayer::Global, BufferrIndexer);
    }
    if(SharedBuffers[EnumToIndex(EConstantBufferLayer::Tick)] == nullptr)
    {
        SharedBuffers[EnumToIndex(EConstantBufferLayer::Tick)] = ParsingBuffer(DX_ShaderReflection, EConstantBufferLayer::Tick, BufferrIndexer);
    }

    BufferrIndexer = constantBufferNum - 1;
    ConstantBuffers[EnumToIndex(EConstantBufferLayer::Object)] = ParsingBuffer(DX_ShaderReflection, EConstantBufferLayer::Object, BufferrIndexer);

	SafeRelease(DX_ShaderReflection);
}

std::shared_ptr<MConstantBuffer> MShader::ParsingBuffer(ID3D11ShaderReflection* InShaderReflection, EConstantBufferLayer InLayer, uint32& BufferrIndexer)
{
    ID3D11ShaderReflectionConstantBuffer* pReflectionConstantBuffer = InShaderReflection->GetConstantBufferByIndex(BufferrIndexer);
    if (nullptr == pReflectionConstantBuffer)
    {
        FAILED_CHECK_THROW(E_FAIL);
    }

    D3D11_SHADER_BUFFER_DESC DX_BufferDesc = {};
    FAILED_CHECK_THROW(pReflectionConstantBuffer->GetDesc(&DX_BufferDesc));

    std::vector<Byte> bufferData(DX_BufferDesc.Size, 0);
    uint32 VariableNum = DX_BufferDesc.Variables;
    std::shared_ptr<MConstantBuffer>& NewConstantBuffer = std::make_shared<MConstantBuffer>(DX_BufferDesc.Size, bufferData.data(), VariableNum);

    D3D11_SHADER_INPUT_BIND_DESC ShaderInputBindDesc = {};
    InShaderReflection->GetResourceBindingDescByName(DX_BufferDesc.Name, &ShaderInputBindDesc);
    uint32 BindPoint = ShaderInputBindDesc.BindPoint;

    uint32 ConstantBufferLayer = EnumToIndex(InLayer);
    if (BindPoint != ConstantBufferLayer)
    {
        return nullptr;
    }

    ++BufferrIndexer;

    for (uint32 VariableIndex = 0; VariableIndex < VariableNum; ++VariableIndex)
    {
        ID3D11ShaderReflectionVariable* pReflectionVariable = pReflectionConstantBuffer->GetVariableByIndex(VariableIndex);
        if (nullptr == pReflectionVariable)
        {
            FAILED_CHECK_THROW(E_FAIL);
        }

        D3D11_SHADER_VARIABLE_DESC variableDesc = { 0 };
        FAILED_CHECK_THROW(pReflectionVariable->GetDesc(&variableDesc));

        if (nullptr != variableDesc.DefaultValue)
        {
            memcpy(bufferData.data() + variableDesc.StartOffset, variableDesc.DefaultValue, variableDesc.Size);
        }

        switch (DX_BufferDesc.Type)
        {
        case D3D11_CBUFFER_TYPE::D3D11_CT_TBUFFER: // 텍스쳐 버퍼
        {
            break;
        }
        case D3D11_CBUFFER_TYPE::D3D11_CT_CBUFFER: // 상수 버퍼
        {
            break;
        }
        }

        std::wstring VariableName;
        StringToWString(variableDesc.Name, VariableName);

        FBufferVariableInfo NewVarbleInfo;
        NewVarbleInfo.Layer = InLayer;
        NewVarbleInfo.Index = VariableIndex;

        FBufferVariable NewVariable(variableDesc.StartOffset, variableDesc.Size);

        NewConstantBuffer->AddVariable(VariableName, NewVarbleInfo, NewVariable);
    }

    //ConstantBuffers[BindPoint] = std::make_shared<MConstantBuffer>(bufferDesc.Size, bufferData.data(), bufferDesc.Variables);
    return NewConstantBuffer;
}

const uint32 MShader::getVariableCountOfConstantBuffer(const EConstantBufferLayer layer)
{
	return ConstantBuffers[CastValue<uint32>(layer)]->getCountOfVariables();
}

std::shared_ptr<MConstantBuffer> MShader::GetConstantBuffer()
{
    return ConstantBuffers[EnumToIndex(EConstantBufferLayer::Object)];
}

ID3D10Blob* MShader::getBlob()
{
	return _pBlob;
}

std::vector<std::vector<FBufferVariable>>& MShader::GetVariables()
{
	return Variables;
}

std::shared_ptr<MConstantBuffer>& MShader::GetSharedConstantBuffer(EConstantBufferLayer InLayer)
{
    return SharedBuffers[EnumToIndex(InLayer)];
}
