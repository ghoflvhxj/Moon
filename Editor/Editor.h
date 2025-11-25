#pragma once

#include "Module/Module.h"
#include "Core/Serialize/JsonSerializer.h"
#include "Core/Serialize/JsonDeSerializer.h"

// 파일 다이얼로그
#include <commdlg.h>

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
    std::unordered_map<std::string, std::shared_ptr<class MAssetEditor>> Editors;

    REFLECT(MEditor)
};

void DispatchContainer(const FTypeDesc* InElementTypeDesc, FVectorPropertyDesc* InContainerDesc, void* InObject);
void DispatchArray(const FTypeDesc* InElementTypeDesc, FPropertyDesc* InPropertyDesc, void* InObject);
void DispatchStruct(const FTypeDesc* InStructDesc, void* InObject);
void HandleProperty(EType InType, const char* DisplayName, void* InData);

void OpenAssetEditor(MObject* InObject, const FTypeDesc* InAssetTypeDesc, const std::wstring& InPath);
