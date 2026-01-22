#pragma once

#include "LightComponent.h"

class MTexture;

class ENGINE_DLL MDirectionalLightComponent : public MLightComponent
{
public:
	explicit MDirectionalLightComponent(void);
	virtual ~MDirectionalLightComponent(void);

public:
    virtual void Update(const Time deltaTime) override;
	virtual const bool GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList) override;

public:
    const Vec3& GetAmbient() const { return Ambient; }
protected:
    Vec3 Ambient = VEC3ONE;

    REFLECT(MDirectionalLightComponent
        , PROPERTY(Ambient)
    )
};
