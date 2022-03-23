#pragma once
#ifndef __COMPONENT_H__

class MainGame;

class ENGINE_DLL Component abstract
{
public:
	explicit Component();
	virtual ~Component();

public:
	std::weak_ptr<MainGame> getOwningGame();
private:
	std::weak_ptr<MainGame> _pOwningGame;
};

#define __COMPONENT_H__
#endif