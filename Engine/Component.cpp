#include "stdafx.h"
#include "Component.h"

Component::Component()
{
}


Component::~Component()
{
}

std::weak_ptr<MainGame> Component::getOwningGame()
{
    return _pOwningGame;
}