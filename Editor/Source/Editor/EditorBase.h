#pragma once

#include "Editor.h"
#include "Core/Delegate.h"

class MAsset;
class Renderer;
class MJoltPhysics;

class MAssetEditor
{
public:
    MAssetEditor(void* InObject, const FTypeDesc* InTypeDesc, std::shared_ptr<MAsset>& InAsset);

public:
    std::shared_ptr<MRenderer> GetRenderer() { return WeakRenderer.lock(); }
    std::shared_ptr< MJoltPhysics> GetJolt() { return WeakJolt.lock(); }
protected:
    std::weak_ptr<MRenderer> WeakRenderer;
    std::weak_ptr<MJoltPhysics> WeakJolt;

public:
    void Update();

public:
    const std::string& GetTitle() const { return Title; }
protected:
    // ImGui 타이틀에 출력될 문자열
    std::string Title;
    // ImGui 닫기 버튼 처리를 위한 변수
    bool bOpen = true;
    // 애셋을 들고있는 오브젝트
    void* Object = nullptr;
    const FTypeDesc* AssetTypeDesc = nullptr;
    std::shared_ptr<MAsset> Asset = nullptr;

public:
    FDelegate<void>& GetClosedDelegate() { return OnClosedDelegate; }
protected:
    FDelegate<void> OnClosedDelegate;
};