#pragma once
#ifndef __FACTORY_H__

#include <memory>

template <class T>
std::shared_ptr<T> CreateDefaultSubObject()
{
	//std::shared_ptr<T> pSubObject = std::make_shared<T>();
	return std::make_shared<T>();
}

#define __FACTORY_H__
#endif