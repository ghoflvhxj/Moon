#pragma once

#include "Jolt.h"

class ENGINE_DLL MDynamicMeshPhysics : public MPhysics
{
public:
    MDynamicMeshPhysics();
    virtual ~MDynamicMeshPhysics();

public:
    FBodyCapsuleData& MakeCapsule(uint32 InJointIndex);
    const std::vector<FBodyCapsuleData>& GetCapsules() const;
    std::vector<FBodyCapsuleData>& GetCapsules();
protected:
    std::vector<FBodyCapsuleData> BodyCapsules;

    REFLECT(
        MDynamicMeshPhysics
        , PROPERTY(BodyCapsules)
    )
};