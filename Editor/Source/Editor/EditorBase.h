#pragma once

#include "Editor.h"
#include "Core/Delegate.h"

class MAsset;
class Renderer;
class MJoltPhysics;

class MAssetEditor
{
public:
    MAssetEditor(const FTypeDesc* InTypeDesc, std::shared_ptr<MAsset>& InAsset);

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
    std::string Title;
    bool bOpen = true;
    const FTypeDesc* AssetTypeDesc = nullptr;
    std::shared_ptr<MAsset> Asset = nullptr;

public:
    FDelegate<void>& GetClosedDelegate() { return OnClosedDelegate; }
protected:
    FDelegate<void> OnClosedDelegate;
};