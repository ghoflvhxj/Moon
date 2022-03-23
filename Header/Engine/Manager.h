#pragma once
#ifndef __MANAGER_H__

template <class T>
class ENGINE_DLL Manager abstract
{
public:
	virtual ~Manager() = default;
protected:
	explicit Manager() = default;
private:
	Manager(const Manager &ref) = delete;
	Manager(Manager &&ref) = delete;
		
};

#define __MANAGER_H__
#endif