#pragma once

#include "Include.h"
#include "Core/Object.h"

class ENGINE_DLL MModule : public MObject
{
public:
    MModule() = default;
    virtual ~MModule();

public:
    virtual bool Initialize();
    virtual void Update() {};
    virtual void Render() {};
    virtual void Release();
    
public:
    bool IsReleased() const { return bReleased; }
    bool IsManualReleaseRequired() const { return bManualReleaseRequired; }
protected:
    bool bReleased = false;
    bool bManualReleaseRequired = false;

public:
    const std::wstring& GetName() const { return ModuleName; }
protected:
    std::wstring ModuleName;

    REFLECT(MModule)
};