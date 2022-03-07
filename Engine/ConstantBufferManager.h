#pragma once
#ifndef __CONSTANT_BUFFER_MANAGER_H__

#include "Singleton.h"

class ConstantBuffer;

class ENGINE_DLL ConstantBufferManager : public Singleton::CSingleton<ConstantBufferManager>
{
private:
	enum class UPDATE_PERIOD
	{
		Constant,
		Frame,
		Object
	};

private:
	static const uint32 MAX_CONSTANTBUFFER_COUNT;
public:
	static const uint32 getMaxBufferCount();

	//-----------------------------------------------------------------------

protected:
	ConstantBufferManager();
private:
	void initialize();

public:
	void updatePerFrame();
	void updatePerObject();
	void updatePerMaterial();

public:
	void addConstantBuffer(LPCSTR name, std::shared_ptr<ConstantBuffer> &pBuffer);
private:
	std::unordered_map<std::string, std::shared_ptr<ConstantBuffer>> _constantBufferMap;
	std::unordered_map<std::string, std::
};

#define __CONSTANT_BUFFER_MANAGER_H__
#endif