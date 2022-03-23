#pragma once
#ifndef __SINGLETON_H__

#include "Include.h"

namespace Singleton
{
	template <class T>
	class CSingleton abstract
	{
	public:
		virtual ~CSingleton() = default;
		CSingleton(const CSingleton &ref) = delete;
		CSingleton(CSingleton &&ref) = delete;
		CSingleton& operator=(const CSingleton &ref) = delete;

	protected:
		CSingleton() = default;

		//----------------------------------------------------------------
	public:
		static T* GetInstance();
	};

	template<class T>
	T* CSingleton<T>::GetInstance()
	{
		static T pInstance;
		return &pInstance;
	}
}

#define __SINGLETON_H__
#endif