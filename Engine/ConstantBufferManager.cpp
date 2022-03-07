#include "stdafx.h"
#include "ConstantBufferManager.h"

#include "ConstantBuffer.h"
#include "ConstantBufferStruct.h"

#include "MainGameSetting.h"

#include "MapUtility.h"

const uint32 ConstantBufferManager::MAX_CONSTANTBUFFER_COUNT = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;

const uint32 ConstantBufferManager::getMaxBufferCount()
{
	return MAX_CONSTANTBUFFER_COUNT;
}

ConstantBufferManager::ConstantBufferManager()
	: Singleton::CSingleton<ConstantBufferManager>()
{
	initialize();
}

void ConstantBufferManager::initialize()
{
	//_constantBufferList.reserve(static_cast<size_t>(getMaxBufferCount()));
	//updatePerFrame();
	////_constantBufferList[enumToIndex(UPDATE_PERIOD::Constant)] = std::make_shared<ConstantBuffer>();
}

void ConstantBufferManager::updatePerFrame()
{
	//ConstantBufferStruct::Frame::Frame frame;
	//frame.width = g_pSetting->getResolutionWidth();
	//frame.height = g_pSetting->getResolutionHeight();

	//_constantBufferList[enumToIndex(UPDATE_PERIOD::Frame)] = std::make_shared<ConstantBuffer>(sizeof(Frame), &frame);
}

void ConstantBufferManager::updatePerObject()
{

}

void ConstantBufferManager::updatePerMaterial()
{

}

void ConstantBufferManager::addConstantBuffer(LPCSTR name, std::shared_ptr<ConstantBuffer> &pBuffer)
{
	MapUtility::FindInsert(_constantBufferMap, name, pBuffer);
}
