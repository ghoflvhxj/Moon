#include "CharacterPhysics.h"

MDynamicMeshPhysics::MDynamicMeshPhysics()
{
    LOGTEXT(TEXT("MDynamicMeshPhysics::Construct"));
}

MDynamicMeshPhysics::~MDynamicMeshPhysics()
{
    LOGTEXT(TEXT("MDynamicMeshPhysics::Destruct"));
}

FBodyCapsuleData& MDynamicMeshPhysics::MakeCapsule(uint32 InJointIndex)
{
    FBodyCapsuleData NewBodyCapsule = {};
    NewBodyCapsule.AttachJointIndex = InJointIndex;
    BodyCapsules.push_back(NewBodyCapsule);
    return BodyCapsules.back();
}

const std::vector<FBodyCapsuleData>& MDynamicMeshPhysics::GetCapsules() const
{
    return BodyCapsules;
}

std::vector<FBodyCapsuleData>& MDynamicMeshPhysics::GetCapsules()
{
    return BodyCapsules;
}
