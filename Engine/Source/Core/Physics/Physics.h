#pragma once

#include "include.h"
#include "PhysicsEnum.h"

class StaticMesh;

class ENGINE_DLL MPhysics
{
public:
    MPhysics() = default;
    ~MPhysics() = default;

public:
    virtual void AddPhysicsObject(std::shared_ptr<StaticMesh> InMesh, EPhysicsType InPhysicsType) = 0;
};