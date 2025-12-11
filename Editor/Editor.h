#pragma once

#include "Module/Module.h"
#include "Core/Serialize/JsonSerializer.h"
#include "Core/Serialize/JsonDeSerializer.h"

// 파일 다이얼로그
#include <commdlg.h>

#include "Core/Delegate.h"

class Component;
class MMeshComponent;
class TerrainComponent;
class SphereComponent;
class MSceneComponent;
class StaticMeshComponent;
class MStaticMeshActor;

class MCamera;
class MActor;
class Player;
class MAsset;
class StaticMesh;

class MEditorBase;

enum class EGizmoMode
{
    Trans,
    Rot,
    Scale,
    Count
};

class MEditor : public MModule
{
public:
	explicit MEditor();
	virtual ~MEditor();

public:
	virtual bool Initialize() override;
    virtual void Release() override;
	virtual void Update() override;
	virtual void Render() override;

public:
    void Open(std::function<void(const TCHAR* InFileName)> InFunction)
    {
        TCHAR FileName[256] = {};
        OPENFILENAMEW OpenFileDesc = {};
        OpenFileDesc.lStructSize = sizeof(OpenFileDesc);
        OpenFileDesc.lpstrFilter = TEXT("json 파일\0*.json");
        OpenFileDesc.lpstrFile = FileName;
        OpenFileDesc.nMaxFile = MAX_PATH;
        OpenFileDesc.lpstrInitialDir = TEXT(".");
        OpenFileDesc.lpstrTitle = TEXT("파일 열기");

        if (GetOpenFileNameW(&OpenFileDesc))
        {
            InFunction(FileName);
        }
    }
    template <class T>
    void Save(T& InObject)
    {
        SaveInternal(InObject, InObject->GetAssetPath());
    }
    template <class T>
    void SaveAs(T& InObject)
    {
        TCHAR FileName[256] = {};
        OPENFILENAMEW OpenFileDesc = {};
        OpenFileDesc.lStructSize = sizeof(OPENFILENAMEW);
        OpenFileDesc.lpstrFilter = TEXT("json파일\0*.json\0");
        OpenFileDesc.lpstrFile = FileName;
        OpenFileDesc.nMaxFile = MAX_PATH;
        OpenFileDesc.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
        OpenFileDesc.lpstrDefExt = TEXT("json");
        if (GetSaveFileNameW(&OpenFileDesc))
        {
            SaveInternal(InObject, FileName);
        }
    }
    template <class T>
    void SaveInternal(T& InObject, const std::wstring& InPath)
    {
        MJsonSerializer Serializer;
        Serializer.Serialize(InObject, InPath, true);
    }

public:
    bool IsPickable() const;
    void SetGizmoMode(EGizmoMode InGizmoMode) { GizmoMode = InGizmoMode; }
    EGizmoMode GetGizmoMode() const { return GizmoMode; }
private:
    std::shared_ptr<StaticMesh> GizmoMesh = nullptr;
    Vec3 GizmoOffset = VEC3ZERO;
    EAxis GizmoAxis;
    bool bSetGizmoOffset = false;
	bool bControlGizmo = false;
    EGizmoMode GizmoMode = EGizmoMode::Trans;
    Vec3 Prev = {};

public:
    void SetClickedComp(std::shared_ptr<MSceneComponent>& InComp);
    const std::shared_ptr<MSceneComponent> GetClickedComp() const { return ClickedComp.lock(); }
    void OnClickedCompChanged();
protected:
    std::weak_ptr<MSceneComponent> ClickedComp;

public:
    void OutLine(MActor* InActor, bool bOutLine);
    float CameraSpeedScale = 1.f;
    Vec3 CurrentRot = {};

public:
    std::unordered_map<std::string, std::shared_ptr<MEditorBase>> Editors;

    REFLECT(MEditor)
};

void DispatchContainer(const FTypeDesc* InElementTypeDesc, FVectorPropertyDesc* InContainerDesc, void* InObject);
void DispatchArray(const FTypeDesc* InElementTypeDesc, FPropertyDesc* InPropertyDesc, void* InObject);
void DispatchType(const FTypeDesc* InTypeDesc, void* InObject);
void DispatchType2(const FTypeDesc* InTypeDesc, void* InData);
void PropertyUI(EType InType, const char* DisplayName, void* InData);

void OpenEditor(MObject* InOwner, std::shared_ptr<MObject> InObject, const std::wstring& InPath);
void OpenEditor(std::shared_ptr<MObject> InObject);

class MEditorBase
{
public:
    MEditorBase();
    virtual ~MEditorBase() = default;

public:
    virtual const std::wstring& GetPath() const { return TEXT(""); }
    virtual void Update() {}
    virtual void RenderUI();
    virtual void HandleObject() {}
    virtual void OnSaved() {}
    bool SetObject(std::shared_ptr<MObject> InObject);

public:
    std::shared_ptr<MRenderer> GetRenderer() { return WeakRenderer.lock(); }
protected:
    std::weak_ptr<MRenderer> WeakRenderer;

public:
    const std::string& GetTitle() const { return Title; }
protected:
    // ImGui 타이틀에 출력될 문자열
    std::string Title;
    // ImGui 닫기 버튼 처리를 위한 변수
    bool bOpen = true;

protected:
    std::shared_ptr<MObject> SourceObject = nullptr;
    std::shared_ptr<MObject> WorkingObject = nullptr;

public:
    FDelegate<void>& GetClosedDelegate() { return OnClosedDelegate; }
protected:
    FDelegate<void> OnClosedDelegate;

protected:
    std::shared_ptr<class MEditorBaseWindow> T = nullptr;
    std::shared_ptr<MWorld> W = nullptr;
    std::shared_ptr<MActor> Light = nullptr;
};