#include "stdafx.h"
#include "ShaderLoader.h"

#include "GraphicDevice.h"

#include "ShaderManager.h"

#include "FileFinder.h"

#include "Thread.h"

ShaderLoader::ShaderLoader()
{
}

ShaderLoader::~ShaderLoader()
{
}

const bool ShaderLoader::loadShader(std::shared_ptr<ShaderManager> shaderManager)
{
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
	}

	return true;
}
