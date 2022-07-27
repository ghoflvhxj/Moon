#include "stdafx.h"
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

Shader::Shader(const std::wstring &filePathName)
	: _constantBuffers(CastValue<uint32>(ConstantBuffersLayer::Count), nullptr)
	, _variableInfos(CastValue<uint32>(ConstantBuffersLayer::Count), std::vector<VariableInfo>())
{
	FAILED_CHECK_THROW(D3DReadFileToBlob(filePathName.c_str(), &_pBlob));
	MakeCosntantBuffers();
}

Shader::~Shader()
{
	SafeRelease(_pBlob);
}

void Shader::UpdateConstantBuffer(const ConstantBuffersLayer layer, std::vector<VariableInfo> &varialbeInfos)
{
	uint32 index = CastValue<uint32>(layer);
	if (nullptr == _constantBuffers[index])
	{
		return;
	}

	uint32 size = _constantBuffers[index]->getSize();
	Byte *pData = (Byte*)_aligned_malloc(size, 16);
	for (VariableInfo &varialbeInfo : varialbeInfos)
	{
		memcpy(pData + varialbeInfo._offset, varialbeInfo._pValue, varialbeInfo._size);
	}

	_constantBuffers[index]->update(pData, size);

	_aligned_free((void*)pData);
}

void Shader::MakeCosntantBuffers()
{
	ID3D11ShaderReflection *pShaderReflection = nullptr;

	FAILED_CHECK_THROW(D3DReflect(_pBlob->GetBufferPointer(), _pBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pShaderReflection));

	D3D11_SHADER_DESC shaderDesc = { 0 };
	FAILED_CHECK_THROW(pShaderReflection->GetDesc(&shaderDesc));

	uint32 constantBufferCount = static_cast<uint32>(shaderDesc.ConstantBuffers);
	if (constantBufferCount > CastValue<uint32>(ConstantBuffersLayer::Count))
	{
		DEV_ASSERT_MSG("ConstantBuffer의 개수가 ConstantBuffersLayer::Countf를 넘어섭니다.");
		return;
	}

	_constantBuffers.reserve(CastValue<size_t>(ConstantBuffersLayer::Count));
	for (uint32 constantBufferIndex = 0; constantBufferIndex < constantBufferCount; ++constantBufferIndex)
	{
		ID3D11ShaderReflectionConstantBuffer *pReflectionConstantBuffer = pShaderReflection->GetConstantBufferByIndex(constantBufferIndex);
		if (nullptr == pReflectionConstantBuffer)
		{
			FAILED_CHECK_THROW(E_FAIL);
		}

		D3D11_SHADER_BUFFER_DESC bufferDesc = {};
		FAILED_CHECK_THROW(pReflectionConstantBuffer->GetDesc(&bufferDesc));
		std::vector<Byte> bufferData(bufferDesc.Size, 0);

		D3D11_SHADER_INPUT_BIND_DESC bindDesc = {};
		pShaderReflection->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc);
		uint32 slotIndex = bindDesc.BindPoint;

		uint32 variableCount = bufferDesc.Variables;
		_variableInfos[slotIndex].reserve(variableCount);
		for (uint32 variableIndex = 0; variableIndex < variableCount; ++variableIndex)
		{
			ID3D11ShaderReflectionVariable *pReflectionVariable = pReflectionConstantBuffer->GetVariableByIndex(variableIndex);
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

			_variableInfos[slotIndex].emplace_back(variableDesc.StartOffset, variableDesc.Size);
		}
		
		std::shared_ptr<ConstantBuffer> pConstantBuffer = std::make_shared<ConstantBuffer>(bufferDesc.Size, bufferData.data(), bufferDesc.Variables);
		_constantBuffers[slotIndex] = pConstantBuffer;
	}
}

const uint32 Shader::getVariableCountOfConstantBuffer(const ConstantBuffersLayer layer)
{
	return _constantBuffers[CastValue<uint32>(layer)]->getCountOfVariables();
}

ID3D10Blob* Shader::getBlob()
{
	return _pBlob;
}

std::vector<std::vector<VariableInfo>>& Shader::getVariableInfos()
{
	return _variableInfos;
}
