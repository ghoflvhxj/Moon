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
    if (GetConstantBuffer(EConstantBufferLayer::Object))
    {
        GetConstantBuffer(EConstantBufferLayer::Object)->Commit();
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

    if (HasConstantBuffer(EConstantBufferLayer::Object))
    {
        Buffers.push_back(GetConstantBuffer(EConstantBufferLayer::Object)->getRaw());
    }
    else
    {
        Buffers.push_back(nullptr);
    }

    return Buffers;
}

//void MShader::UpdateConstantBuffer(const EConstantBufferLayer layer, std::vector<FBufferVariable>& InVariables)
//{
//	uint32 Index = CastValue<uint32>(layer);
//    
//    for (int i=0; i< Variables[Index].size(); ++i)
//    {
//        Variables[Index][i] = InVariables[i];
//    }
//
//    UpdateConstantBuffer(layer);
//}

//void MShader::UpdateConstantBuffer(const EConstantBufferLayer layer)
//{
//	if (HasConstantBuffer(layer) == false)
//	{
//		return;
//	}
//
//	uint32 Index = CastValue<uint32>(layer);
//	for (const FBufferVariable& Variable : Variables[Index])
//	{
//		ConstantBuffers[Index]->SetData(Variable.Offset, Variable.Value, Variable.Size);
//	}
//}

void MShader::CreateCosntantBuffers()
{
	ID3D11ShaderReflection *DX_ShaderReflection = nullptr;

	FAILED_CHECK_THROW(D3DReflect(_pBlob->GetBufferPointer(), _pBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&DX_ShaderReflection));

	D3D11_SHADER_DESC DX_ShaderDesc = { 0 };
	FAILED_CHECK_THROW(DX_ShaderReflection->GetDesc(&DX_ShaderDesc));

	uint32 ConstantBufferLayerNum = CastValue<uint32>(EConstantBufferLayer::Count);
	uint32 BufferNum = static_cast<uint32>(DX_ShaderDesc.ConstantBuffers);
	if (BufferNum > ConstantBufferLayerNum)
	{
		DEV_ASSERT_MSG("ConstantBuffer의 개수가 ConstantBuffersLayer::Countf를 넘어섭니다.");
	}

    if (BufferNum == 0)
    {
        return;
    }

    for (uint32 BufferIndex = 0; BufferIndex < BufferNum; ++BufferIndex)
    {
        EConstantBufferLayer BufferLayer = EConstantBufferLayer::Count;
        std::shared_ptr<MConstantBuffer> NewBuffer = ParseBuffer(DX_ShaderReflection, BufferIndex, BufferLayer);

        if (BufferLayer == EConstantBufferLayer::Count)
        {
            continue;
        }

        switch (BufferLayer)
        {
        case EConstantBufferLayer::Global:
        case EConstantBufferLayer::Tick:
        {
            auto& BufferSlot = SharedBuffers[EnumToIndex(BufferLayer)];
            if (BufferSlot == nullptr)
            {
                BufferSlot = NewBuffer;
            }
        }
        break;
        case EConstantBufferLayer::Object:
        case EConstantBufferLayer::Custom:
        {
            ConstantBuffers[EnumToIndex(BufferLayer)] = NewBuffer;
        }
        break;
        }
    }

	SafeRelease(DX_ShaderReflection);
}

std::shared_ptr<MConstantBuffer> MShader::ParseBuffer(ID3D11ShaderReflection* InShaderReflection, uint32 InBufferIndex, EConstantBufferLayer& OutLayer)
{
    /*********************************************
     cbuffer 사용 여부에 따라서 cbuffer개수와 인덱스가 정해짐.
     총 A, B, C 레이어가 있는데 MyShader.hlsl에서 B만 사용한다면
     B의 정보만 저장되있고 해당 인덱스는 0이 됨
    *********************************************/
    ID3D11ShaderReflectionConstantBuffer* pReflectionConstantBuffer = InShaderReflection->GetConstantBufferByIndex(static_cast<UINT>(InBufferIndex));
    assert(pReflectionConstantBuffer);

    D3D11_SHADER_BUFFER_DESC DX_BufferDesc = {};
    FAILED_CHECK_THROW(pReflectionConstantBuffer->GetDesc(&DX_BufferDesc));

    if (DX_BufferDesc.Type == D3D_CBUFFER_TYPE::D3D11_CT_RESOURCE_BIND_INFO)
    {
        return nullptr;
    }

    std::vector<Byte> bufferData(DX_BufferDesc.Size, 0);
    uint32 VariableNum = DX_BufferDesc.Variables;
    std::shared_ptr<MConstantBuffer>& NewConstantBuffer = std::make_shared<MConstantBuffer>(DX_BufferDesc.Size, bufferData.data(), VariableNum);

    D3D11_SHADER_INPUT_BIND_DESC ShaderInputBindDesc = {};
    InShaderReflection->GetResourceBindingDescByName(DX_BufferDesc.Name, &ShaderInputBindDesc);

    OutLayer = static_cast<EConstantBufferLayer>(ShaderInputBindDesc.BindPoint);

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
        NewVarbleInfo.Layer = OutLayer;
        NewVarbleInfo.Index = VariableIndex;

        FBufferVariable NewVariable(variableDesc.StartOffset, variableDesc.Size);

        NewConstantBuffer->AddVariable(VariableName, NewVarbleInfo, NewVariable);
        VariableInfos.emplace(VariableName, NewVarbleInfo);
    }

    //ConstantBuffers[BindPoint] = std::make_shared<MConstantBuffer>(bufferDesc.Size, bufferData.data(), bufferDesc.Variables);
    return NewConstantBuffer;
}

const uint32 MShader::getVariableCountOfConstantBuffer(const EConstantBufferLayer layer)
{
	return ConstantBuffers[CastValue<uint32>(layer)]->getCountOfVariables();
}

std::shared_ptr<MConstantBuffer> MShader::GetConstantBuffer(EConstantBufferLayer InLayer)
{
    return ConstantBuffers[EnumToIndex(InLayer)];
}

bool MShader::HasConstantBuffer(EConstantBufferLayer InLayer)
{
    return GetConstantBuffer(InLayer) != nullptr;
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
