#include "stdafx.h"
#include "ShaderLoader.h"

#include "GraphicDevice.h"
#include "ConstantBuffer.h"

#include "ShaderManager.h"

#include "FileFinder.h"

#include "Thread.h"
#include <thread>

ShaderLoader::ShaderLoader()
{
}

ShaderLoader::~ShaderLoader()
{
}

const bool ShaderLoader::loadShader(std::shared_ptr<ShaderManager> shaderManager)
{
	// 멀티 쓰레드로 변경하기
	loadVertexShaderFromFiles(shaderManager);
	loadPixelShaderFromFiles(shaderManager);

	return true;
}

const bool ShaderLoader::loadVertexShaderFromFiles(std::shared_ptr<ShaderManager> shaderManager)
{
	OutputDebugString(TEXT("VertexShader 불러오는 중..."));

	WCHAR path[MAX_PATH] = {};
	getVertexShaderDirectory(path);
	FileFinder fileFinder(path);

	ID3D11Device *pDevice = g_pGraphicDevice->getDevice();

	auto &fileList = fileFinder.getFileList();
	for (auto &filePathName : fileList) 
	{
		ID3D11VertexShader *pShader = nullptr;
		ID3D10Blob *pBlob = nullptr;

		FAILED_CHECK_THROW(D3DReadFileToBlob(filePathName.c_str(), &pBlob));
		FAILED_CHECK_THROW(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShader));

		WCHAR fileName[MAX_PATH] = {};
		lstrcpy(fileName, PathFindFileName(filePathName.c_str()));

		shaderManager->addVertexShader(fileName, pShader, pBlob);
		extractConstantBuffer(pBlob);
	}

	return true;
}

const bool ShaderLoader::loadPixelShaderFromFiles(std::shared_ptr<ShaderManager> shaderManager)
{
	OutputDebugString(TEXT("PixelShader 불러오는 중..."));

	WCHAR path[MAX_PATH];
	getPixelShaderDirectory(path);
	FileFinder fileFinder(path);

	ID3D11Device *pDevice = g_pGraphicDevice->getDevice();

	auto &fileList = fileFinder.getFileList();
	for (auto &filePathName : fileList)
	{
		ID3D11PixelShader *pShader = nullptr;
		ID3D10Blob *pBlob = nullptr;

		FAILED_CHECK_THROW(D3DReadFileToBlob(filePathName.c_str(), &pBlob));
		FAILED_CHECK_THROW(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pShader));

		WCHAR fileName[MAX_PATH] = {};
		lstrcpy(fileName, PathFindFileName(filePathName.c_str()));

		shaderManager->addPixelShader(fileName, pShader, pBlob);
		extractConstantBuffer(pBlob);
	}

	return true;
}

const bool ShaderLoader::extractConstantBuffer(ID3D10Blob *pBlob)
{
	ID3D11ShaderReflection *pShaderReflection = nullptr;

	FAILED_CHECK_THROW(D3DReflect(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pShaderReflection));

	D3D11_SHADER_DESC shaderDesc = { 0 };
	FAILED_CHECK_THROW(pShaderReflection->GetDesc(&shaderDesc));

	uint32 constantBufferCount = static_cast<uint32>(shaderDesc.ConstantBuffers);
	for (uint32 constantBufferIndex = 0; constantBufferIndex < constantBufferCount; ++constantBufferIndex)
	{
		ID3D11ShaderReflectionConstantBuffer *pReflectionConstantBuffer = nullptr;
		pReflectionConstantBuffer = pShaderReflection->GetConstantBufferByIndex(constantBufferIndex);		
		if (nullptr == pReflectionConstantBuffer)
		{
			FAILED_CHECK_THROW(E_FAIL);
		}

		D3D11_SHADER_BUFFER_DESC bufferDesc = { 0 };
		FAILED_CHECK_THROW(pReflectionConstantBuffer->GetDesc(&bufferDesc));

		std::vector<Byte> bufferData(bufferDesc.Size, 0);
		std::shared_ptr<ConstantBuffer> pConstantBuffer = std::make_shared<ConstantBuffer>(bufferDesc.Size, bufferData.data());	// 여기서 상수버퍼라고 확정지었는데, 이러지 말고 상수버퍼가 버퍼를 상속받게 만들고, 버퍼 클래스 추가
		uint32 variableCount = bufferDesc.Variables;
				
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
					//ConstantBufferManager::GetInstance()->addConstantBuffer(bufferDesc.Name, pConstantBuffer);
					break;
				}
			}
		}
				
		// =  pShaderReflectionConstantBuffer->GetVariableByName("test");
	}
}
