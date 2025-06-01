#include "Include.h"
#include "Shader.h"

#include "GraphicDevice.h"
#include "ConstantBuffer.h"

#include <memory.h>

//void Shader::ExtractConstantBufferVariables(std::shared_ptr<Shader> pShader, VariableInfoOfConstantBuffers &variableInfoOfConstantBuffers)
//{
//	variableInfoOfConstantBuffers.clear();
//
//	ID3D11ShaderReflection *pShaderReflection = nullptr;
//	ID3D10Blob *pBlob = pShader->getBlob();
//	FAILED_CHECK_THROW(D3DReflect(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pShaderReflection));
//
//	D3D11_SHADER_DESC shaderDesc = { 0 };
//	FAILED_CHECK_THROW(pShaderReflection->GetDesc(&shaderDesc));
//
//	uint32 constantBufferCount = static_cast<uint32>(shaderDesc.ConstantBuffers);
//	_variableInfo.reserve(constantBufferCount);
//	for (uint32 constantBufferIndex = 0; constantBufferIndex < constantBufferCount; ++constantBufferIndex)
//	{
//		variableInfoOfConstantBuffers.emplace_back(std::vector<VariableInfo>());
//
//		ID3D11ShaderReflectionConstantBuffer *pReflectionConstantBuffer = nullptr;
//		pReflectionConstantBuffer = pShaderReflection->GetConstantBufferByIndex(constantBufferIndex);
//		if (nullptr == pReflectionConstantBuffer)
//		{
//			FAILED_CHECK_THROW(E_FAIL);
//		}
//
//		D3D11_SHADER_BUFFER_DESC bufferDesc = { 0 };
//		FAILED_CHECK_THROW(pReflectionConstantBuffer->GetDesc(&bufferDesc));
//
//		std::vector<Byte> bufferData(bufferDesc.Size, 0);
//		std::shared_ptr<ConstantBuffer> pConstantBuffer = std::make_shared<ConstantBuffer>(bufferDesc.Size, bufferData.data(), bufferDesc.Variables);	// 여기서 상수버퍼라고 확정지었는데, 이러지 말고 상수버퍼가 버퍼를 상속받게 만들고, 버퍼 클래스 추가
//		uint32 variableCount = bufferDesc.Variables;
//
//		for (uint32 variableIndex = 0; variableIndex < variableCount; ++variableIndex)
//		{
//			ID3D11ShaderReflectionVariable *pReflectionVariable = pReflectionConstantBuffer->GetVariableByIndex(variableIndex);
//			if (nullptr == pReflectionVariable)
//			{
//				FAILED_CHECK_THROW(E_FAIL);
//			}
//
//			D3D11_SHADER_VARIABLE_DESC variableDesc = { 0 };
//			FAILED_CHECK_THROW(pReflectionVariable->GetDesc(&variableDesc));
//
//			VariableInfo info = {};
//			info._offset	= variableDesc.StartOffset;
//			info._size		= variableDesc.Size;
//			info._pValue	= new Byte[variableDesc.Size];
//			variableInfoOfConstantBuffers.back().emplace_back(info);
//
//			//if (nullptr != variableDesc.DefaultValue)
//			//{
//			//	memcpy(bufferData.data() + variableDesc.StartOffset, variableDesc.DefaultValue, variableDesc.Size);
//			//}
//			//
//			//switch (bufferDesc.Type)
//			//{
//			//case D3D11_CBUFFER_TYPE::D3D11_CT_TBUFFER: // 텍스쳐 버퍼
//			//{
//			//	break;
//			//}
//			//case D3D11_CBUFFER_TYPE::D3D11_CT_CBUFFER: // 상수 버퍼
//			//{
//			//	break;
//			//}
//			//}
//		}
//	}
//}

MShader::MShader(const std::wstring &filePathName)
	: ConstantBuffers(CastValue<uint32>(EConstantBufferLayer::Count), nullptr)
	, Variables(CastValue<uint32>(EConstantBufferLayer::Count), std::vector<FShaderVariable>())
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

void MShader::UpdateConstantBuffer(const EConstantBufferLayer layer, std::vector<FShaderVariable>& InVariables)
{
	uint32 Index = CastValue<uint32>(layer);
	if (nullptr == ConstantBuffers[Index])
	{
		return;
	}

	uint32 size = ConstantBuffers[Index]->getSize();
	Byte* pData = (Byte*)_aligned_malloc(size, 16);
	for (FShaderVariable& varialbeInfo : InVariables)
	{
		memcpy(pData + varialbeInfo.Offset, varialbeInfo.Value, varialbeInfo.Size);
	}

	ConstantBuffers[Index]->Update(pData);

	_aligned_free((void*)pData);

	//for (const FShaderVariable& Variable : InVariables)
	//{
	//	ConstantBuffers[Index]->SetData(Variable.Offset, Variable.Value, Variable.Size);
	//}

	//ConstantBuffers[Index]->Commit();
}

void MShader::UpdateConstantBuffer(const EConstantBufferLayer layer)
{
	uint32 Index = CastValue<uint32>(layer);
	if (nullptr == ConstantBuffers[Index])
	{
		return;
	}

	for (const FShaderVariable& Variable : Variables[Index])
	{
		ConstantBuffers[Index]->SetData(Variable.Offset, Variable.Value, Variable.Size);
	}

	ConstantBuffers[Index]->Commit();
}

void MShader::CreateCosntantBuffers()
{
	ID3D11ShaderReflection *pShaderReflection = nullptr;

	FAILED_CHECK_THROW(D3DReflect(_pBlob->GetBufferPointer(), _pBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pShaderReflection));

	D3D11_SHADER_DESC shaderDesc = { 0 };
	FAILED_CHECK_THROW(pShaderReflection->GetDesc(&shaderDesc));

	uint32 ConstantBufferLayerNum = CastValue<uint32>(EConstantBufferLayer::Count);
	uint32 constantBufferNum = static_cast<uint32>(shaderDesc.ConstantBuffers);
	if (constantBufferNum > ConstantBufferLayerNum)
	{
		DEV_ASSERT_MSG("ConstantBuffer의 개수가 ConstantBuffersLayer::Countf를 넘어섭니다.");
	}

	for (uint32 ConstantBufferCounter = 0; ConstantBufferCounter < constantBufferNum; ++ConstantBufferCounter)
	{
		ID3D11ShaderReflectionConstantBuffer *pReflectionConstantBuffer = pShaderReflection->GetConstantBufferByIndex(ConstantBufferCounter);
		if (nullptr == pReflectionConstantBuffer)
		{
			FAILED_CHECK_THROW(E_FAIL);
		}

		D3D11_SHADER_BUFFER_DESC bufferDesc = {};
		FAILED_CHECK_THROW(pReflectionConstantBuffer->GetDesc(&bufferDesc));
		std::vector<Byte> bufferData(bufferDesc.Size, 0);
		uint32 variableCount = bufferDesc.Variables;

		D3D11_SHADER_INPUT_BIND_DESC ShaderInputBindDesc = {};
		pShaderReflection->GetResourceBindingDescByName(bufferDesc.Name, &ShaderInputBindDesc);
		uint32 LayerIndex = ShaderInputBindDesc.BindPoint;
		
		for (uint32 VariableIndex = 0; VariableIndex < variableCount; ++VariableIndex)
		{
			ID3D11ShaderReflectionVariable *pReflectionVariable = pReflectionConstantBuffer->GetVariableByIndex(VariableIndex);
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

			switch (bufferDesc.Type)
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

			Variables[LayerIndex].emplace_back(variableDesc.StartOffset, variableDesc.Size);
			FShaderVariableInfo NewVarbleInfo;
			NewVarbleInfo.Layer			= static_cast<EConstantBufferLayer>(LayerIndex);
			NewVarbleInfo.Index			= VariableIndex;	
			VariableInfos[VariableName] = NewVarbleInfo;
		}
		
		ConstantBuffers[LayerIndex] = std::make_shared<MConstantBuffer>(bufferDesc.Size, bufferData.data(), bufferDesc.Variables);
	}

	SafeRelease(pShaderReflection);
}

const uint32 MShader::getVariableCountOfConstantBuffer(const EConstantBufferLayer layer)
{
	return ConstantBuffers[CastValue<uint32>(layer)]->getCountOfVariables();
}

ID3D10Blob* MShader::getBlob()
{
	return _pBlob;
}

std::vector<std::vector<FShaderVariable>>& MShader::GetVariables()
{
	return Variables;
}
