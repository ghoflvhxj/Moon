#pragma once

#include "Actor.h"

class MMeshComponent;
class StaticMeshComponent;
class DynamicMeshComponent;
class TerrainComponent;
class MTexture;
class MPointLightComponent;
class MDirectionalLightComponent;
class SkyComponent;

class Player : public MActor
{
public:
    explicit Player();
    virtual ~Player();

public:
    virtual void BeginPlay() override;
protected:
    virtual void tick(const Time deltaTime) override;

public:
    void JsonSaveTest(bool bPretty = false);
    void JsonLoadTest();

private:
    std::shared_ptr<DynamicMeshComponent>	CharacterMeshComponent;
    std::shared_ptr<MPointLightComponent>	_pLightComponent;
    std::shared_ptr<MDirectionalLightComponent>	_pLightComponent2;

    std::shared_ptr<SkyComponent>			_pSkyComponent;

    // 테스트
protected:
    std::shared_ptr<StaticMeshComponent>	LoadedStaticMeshComp;
    std::shared_ptr<DynamicMeshComponent>   LoadedDynamicMeshComp;
    
    // 제거
private:
    std::shared_ptr<StaticMeshComponent>	_pStaticMeshComponent2;
    std::vector<std::shared_ptr<MPointLightComponent>>	_pLightComponentList;

private:
    float CameraSpeedScale = 1.f;

    REFLECT(Player)
};