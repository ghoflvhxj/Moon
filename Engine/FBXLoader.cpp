#include "FBXLoader.h"
#include "FBXSDK/fbxsdk/scene/fbxaxissystem.h"

#include "Texture.h"
#include "Core/ResourceManager.h"
#include "Core/Serialize/JsonSerializer.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"
#include "Material.h"

#include "Utility/PerformanceTimer.h"

#undef min
#undef max

using namespace std;
using namespace fbxsdk;

FbxManager *MFBXLoader::_pFbxManager = nullptr;

MFBXLoader::MFBXLoader()
	: Path{ TEXT("") }
	, Directory{ TEXT("") }
	, _pImporter{ nullptr }
	, _pScene{ nullptr }
	, _pSkeleton{ nullptr }
	, _pAnimStack{ nullptr }
	, GeometryCount{ 0 }
	, MaterialNum{ 0 }
	, meshCounter{ 0 }
{

}

MFBXLoader::MFBXLoader(const wchar_t* filePathName)
	: Path	{ filePathName }
	, Directory		{ filePathName }
	, _pImporter	{ nullptr }
	, _pScene		{ nullptr }
	, _pSkeleton	{ nullptr }
	, _pAnimStack	{ nullptr }
	, GeometryCount{ 0 }
	, MaterialNum{ 0 }
	, meshCounter{ 0 }
{
	LoadFBXMesh(Path);
}

MFBXLoader::~MFBXLoader()
{
	_pScene->Destroy();
	_pImporter->Destroy();
	//_pFbxManager->Destroy();
}

void MFBXLoader::LoadFBXAnim(std::vector<MAnimation>& OutAnimationClips)
{
    OutAnimationClips.resize(AnimStackNum);

	PerformanceTimer timer;

	for (uint32 meshIndex = 0; meshIndex < GeometryCount; ++meshIndex)
	{
		FbxMesh* pMesh = _meshList[meshIndex];
		FbxNode* pMeshNode = pMesh->GetNode(0);

		FbxAMatrix MeshGlobalTransform = pMeshNode->EvaluateGlobalTransform();
		FbxAMatrix geometryTransform = {
			pMeshNode->GetGeometricTranslation(FbxNode::EPivotSet::eSourcePivot),
			pMeshNode->GetGeometricRotation(FbxNode::EPivotSet::eSourcePivot),
			pMeshNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot)
		};

		int deformerCount = pMesh->GetDeformerCount();
		for (int deformerIndex = 0; deformerIndex < deformerCount; ++deformerIndex)
		{
			FbxDeformer* pDeformer = pMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin);
			if (nullptr == pDeformer)
			{
				continue;
			}

			FbxSkin* pSkin = reinterpret_cast<FbxSkin*>(pDeformer);

			int clusterCount = pSkin->GetClusterCount();
			for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
			{
				FbxCluster* pCluster = pSkin->GetCluster(clusterIndex);
				const char* JointName = pCluster->GetLink()->GetName();
				int JointIndex = NameToJointIndex[JointName];

				MeshJointIndices[meshIndex].push_back(JointIndex);

				std::wstring str = TEXT("JointName: ") + StringToWString(JointName) + TEXT(", JointIndex: ") + std::to_wstring(JointIndex);
				LOG(str);

				// 바인드 포즈 역행렬 = 조인트 역행렬 * 클러스터 행렬
				FbxAMatrix JointTransformMatrix;
				pCluster->GetTransformLinkMatrix(JointTransformMatrix);
				FbxAMatrix ClusterTransformMatrix;
				pCluster->GetTransformMatrix(ClusterTransformMatrix);
				FbxAMatrix globalBindPoseInverseMatrix;
				globalBindPoseInverseMatrix = JointTransformMatrix.Inverse() * ClusterTransformMatrix * geometryTransform;
				//globalBindPoseInverseMatrix = JointTransformMatrix.Inverse() * ClusterTransformMatrix;

				XMStoreFloat4x4(&Joints[JointIndex]._globalBindPoseInverseMatrix, ToXMMatrix(globalBindPoseInverseMatrix));

				auto& BindPose = globalBindPoseInverseMatrix.Inverse();
				auto& Scale = BindPose.GetS();
				auto& Rot = BindPose.GetR();
				auto& Trans = BindPose.GetT();

				Joints[JointIndex].Scale = { (float)Scale[0], (float)Scale[1], (float)Scale[2] };
				Joints[JointIndex].Rotation = { ToRadian((float)Rot[0]), ToRadian((float)Rot[1]), ToRadian((float)Rot[2]) };
				Joints[JointIndex].Position = { (float)Trans[0], (float)Trans[1], (float)Trans[2] };

				// 조인트가 영향을 주는 정점들을 찾아서, 자신의 정보를 저장시킴
				double* ControlPointWeights = pCluster->GetControlPointWeights();
				if (ControlPointWeights != nullptr)
				{
					for (int ArrayIndex = 0; ArrayIndex < pCluster->GetControlPointIndicesCount(); ++ArrayIndex)
					{
						// Weights와 ControlPoindIndices의 수는 4개로 같음
						int ControlPointIndex = pCluster->GetControlPointIndices()[ArrayIndex];
						std::vector<int>& VertexIndices = ControlPointToVertexIndices[meshIndex][ControlPointIndex];

						float Weight = static_cast<float>(ControlPointWeights[ArrayIndex]);

						for (int VertexIndex : VertexIndices)
						{
							// 저장할 위치를 찾기 위한 로직
							for (int BlendCounter = 0; BlendCounter < 4; ++BlendCounter)
							{
								if (_verticesList[meshIndex][VertexIndex].BlendIndex[BlendCounter] == -1)
								{
									_verticesList[meshIndex][VertexIndex].BlendIndex[BlendCounter] = JointIndex;
									_verticesList[meshIndex][VertexIndex].BlendWeight[BlendCounter] = Weight;
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	for (uint32 AnimStackIndex = 0; AnimStackIndex < AnimStackNum; ++AnimStackIndex)
	{
		_pAnimStack = _pScene->GetCurrentAnimationStack();
		FbxString animStackName = _pAnimStack->GetName();
		FbxTakeInfo* pTakeInfo = _pScene->GetTakeInfo(animStackName);

		MAnimation& CurrentAnimClip = OutAnimationClips[AnimStackIndex];
		CurrentAnimClip.Name = animStackName.Buffer();
		CurrentAnimClip.SetAssetPath(Directory + StringToWString(CurrentAnimClip.Name) + TEXT(".json"));

        FbxTime Start = pTakeInfo->mLocalTimeSpan.GetStart();
        FbxTime End = pTakeInfo->mLocalTimeSpan.GetStop();
        float Duration = static_cast<float>((End - Start).GetSecondDouble());
        uint32 StartFrame = CastValue<uint32>(Start.GetFrameCount(TimeMode));
        uint32 EndFrame = CastValue<uint32>(End.GetFrameCount(TimeMode));
		CurrentAnimClip.SetFrameInfo(GetFrameRate(TimeMode), Duration, StartFrame, EndFrame);

		if (std::shared_ptr<DynamicMesh> DM = g_ResourceManager->FindDynamicMesh(Joints))
		{
			auto& MeshDatas = DM->GetMeshDatas();
            for (uint32 meshIndex = 0; meshIndex < GetSize(MeshDatas); ++meshIndex)
            {
                const FMeshData& MeshData = DM->GetMeshData(meshIndex);

                FbxAMatrix MeshGlobalInv;
                for (uint32 i = 0; i < 16; ++i)
                {
                    uint32 Row = i / 4;
                    uint32 Col = i % 4;
                    MeshGlobalInv.mData[Row][Col] = MeshData.GlobalInverseTransform.m[Row][Col];
                }

                for (uint32 JointIndex = 0; JointIndex < GetSize(Joints); JointIndex++)
                {
                    for (uint32 Frame = 0; Frame < CurrentAnimClip.TotalFrame; ++Frame)
                    {
                        FbxTime currentTime;
                        currentTime.SetFrame(static_cast<FbxLongLong>(CurrentAnimClip.StartFrame + Frame), TimeMode);

                        FbxAMatrix JointGlobal = JointNodes[JointIndex]->EvaluateGlobalTransform(currentTime); //pCluster->GetLink()->EvaluateGlobalTransform(currentTime);
                        FbxAMatrix& Test = MeshGlobalInv * JointGlobal;
                        XMStoreFloat4x4(&CurrentAnimClip.GetKeyFrame(Frame).GetJointMatrix(JointIndex), ToXMMatrix(Test));
                    }
                }
            }
		}
	}
}

bool MFBXLoader::LoadFBXMesh(const wstring& InPath)
{
    if (bool bLoaded = Path.empty() == false)
    {
        return true;
    }

	Path = InPath;
    Directory = MFileSystem::GetDirectory(InPath);
    std::filesystem::path PathObject(Path);
    Name = PathObject.filename().wstring();
    Name = Name.substr(0, Name.find_last_of('.'));
    Extension = PathObject.extension();

	InitializeFbxSdk();
	convertScene();

	GeometryCount = _pScene->GetGeometryCount();
	_verticesList.resize(GeometryCount);
	_indicesList.resize(GeometryCount);
    MeshInvGlobalTransforms.resize(GeometryCount, ZEROMATRIX);
    MeshJointIndices.resize(GeometryCount);
	MaterialIndices.reserve(GeometryCount);
	ControlPointToVertexIndices.resize(GeometryCount);

	MaterialNum = static_cast<uint32>(_pScene->GetMaterialCount());
	MaterialTextures.reserve(MaterialNum);

    AnimStackNum = static_cast<uint32>(_pImporter->GetAnimStackCount());

	loadNode();
	loadTexture();

	return false;
}

void MFBXLoader::SaveJsonAsset(const std::wstring& InPath, bool bMesh /*= true*/, bool bMaterial /*= true*/, bool bSkeleton /*= false*/, bool bAnim /*= false*/)
{
    LoadFBXMesh(InPath);

    bool bDynamic = GetSize(Joints) > 0 || AnimStackNum > 0;
    //if (AnimStackNum > 0)
    //{
    //    for (auto& Mesh : _meshList)
    //    {
    //        if (Mesh->GetDeformerCount() > 0)
    //        {
    //            bDynamic = true;
    //            break;
    //        }
    //    }
    //}

    std::shared_ptr<MMesh> NewMesh = nullptr;
    if (bDynamic)
    {
        NewMesh = std::make_shared<DynamicMesh>();
    }
    else
    {
        NewMesh = std::make_shared<StaticMesh>();
    }
    NewMesh->LoadFromFBX(InPath, *this);

    std::set<uint32> UniqueMaterialIndices;
    for (uint32 MaterialIndex : MaterialIndices)
    {
        UniqueMaterialIndices.emplace(MaterialIndex);
    }

    if (bMaterial)
    {
        for (auto& Material : NewMesh->getMaterials())
        {
            MJsonSerializer MatSerializer;
            MatSerializer.Serialize(*Material, Material->GetAssetPath(), true);
        }
    }

    if (bMesh)
    {
        MJsonSerializer Serializer;
        std::wstring MeshPath = Directory + Name + TEXT(".json");
        NewMesh->SetAssetPath(MeshPath);
        Serializer.Serialize(*NewMesh, NewMesh->GetAssetPath(), false);
    }

    if (bDynamic)
    {
        std::shared_ptr<DynamicMesh> NewDynamicMesh = std::static_pointer_cast<DynamicMesh>(NewMesh);
        if (bSkeleton)
        {
            MJsonSerializer Serializer;
            Serializer.Serialize(*(NewDynamicMesh->Skeleton), NewDynamicMesh->Skeleton->GetAssetPath(), true);
        }

        if (bAnim)
        {
            const std::vector<MAnimation>& Anims = NewDynamicMesh->GetAnimClips();
            for (auto& Anim : Anims)
            {
                MJsonSerializer Serializer;
                Serializer.Serialize(Anim, Anim.GetAssetPath(), true);

                // 기존 애셋 갱신
                if (auto OldAsset = g_ResourceManager->FindAsset(Anim.GetAssetPath()))
                {
                    Anim.Copy(OldAsset.get());
                }
            }
        }
    }
}

void MFBXLoader::InitializeFbxSdk()
{
    if (nullptr == _pFbxManager)
    {
        _pFbxManager = FbxManager::Create();

        FbxIOSettings* ios = FbxIOSettings::Create(_pFbxManager, IOSROOT);
        _pFbxManager->SetIOSettings(ios);
    }

	_pImporter = FbxImporter::Create(_pFbxManager, "");
	if (nullptr == _pImporter)
	{
		DEV_ASSERT_MSG("FbxImporter가 nullptr 입니다!");
	}

	char FilePathName[255] = { 0, };
	WStringToString(Path, FilePathName, 255);
	if (false == _pImporter->Initialize(FilePathName))
	{
		DEV_ASSERT_MSG("FbxImporter 초기화에 실패했습니다!");
	}

	_pScene = FbxScene::Create(_pFbxManager, "Scene");
	if (nullptr == _pScene)
	{
		DEV_ASSERT_MSG("FbxScene이 nullptr  입니다!");
	}

	if (false == _pImporter->Import(_pScene))
	{
		DEV_ASSERT_MSG("FbxImporter가 FbxScene을 불러오지 못했습니다!");
	}

    TimeMode = _pScene->GetGlobalSettings().GetTimeMode();
}

void MFBXLoader::convertScene()
{
	FbxAxisSystem directXAxisSys(FbxAxisSystem::EPreDefinedAxisSystem::eDirectX);
	directXAxisSys.DeepConvertScene(_pScene);
	//directXAxisSys.ConvertScene(_pScene);

	FbxGeometryConverter geometryConverter(_pFbxManager);
	geometryConverter.Triangulate(_pScene, true);
}

uint32 MFBXLoader::GetFrameRate(FbxTime::EMode InTimeMode)
{
    switch (InTimeMode)
    {
    case FbxTime::eFrames24:
        return 24;
    case FbxTime::eFrames30:
        return 30;
    }

    return 24;
}

const uint32 MFBXLoader::getJointCount() const
{
	return CastValue<uint32>(Joints.size());
}

const uint32 MFBXLoader::GetGeometryNum() const
{
	return GeometryCount;
}

const uint32 MFBXLoader::GetMaterialNum() const
{
	return MaterialNum;
}

std::wstring MFBXLoader::GetMaterialIName(uint32 Index)
{
    if (_pScene)
    {
        if (FbxSurfaceMaterial* Material = _pScene->GetMaterial(Index))
        {
            return StringToWString(Material->GetName());
        }
    }

    return TEXT("DefulatMaterial");
}

const uint32 MFBXLoader::GetTotalVertexNum() const
{
	return TotalVertexNum;
}

std::vector<VertexList> &MFBXLoader::getVerticesList()
{
	return _verticesList;
}

std::vector<IndexList> &MFBXLoader::getIndicesList()
{
	return _indicesList;
}

std::vector<Mat4>& MFBXLoader::GetMeshInvGlobalTransforms()
{
    return MeshInvGlobalTransforms;
}

std::vector<TextureList> &MFBXLoader::GetTextures()
{
	return MaterialTextures;
}

const std::vector<uint32>& MFBXLoader::GetMaterialIndices() const
{
	return MaterialIndices;
}

void MFBXLoader::loadNode()
{
	std::vector<FbxNode*> nodeList;
	nodeList.reserve(_pScene->GetNodeCount());
	nodeList.push_back(_pScene->GetRootNode());

	while (false == nodeList.empty())
	{
		FbxNode *pNode = nodeList.back();
		FbxNodeAttribute *pNodeAttribute = pNode->GetNodeAttribute();

		if (nullptr != pNodeAttribute)
		{
            FbxNodeAttribute::EType AttributeType = pNodeAttribute->GetAttributeType();
            std::string NodeName = pNode->GetName();
            std::string TypeName = pNode->GetTypeName();
			switch (AttributeType)
			{
			case FbxNodeAttribute::EType::eMesh:
				parseMeshNode(pNode, meshCounter++);
				break;
			case FbxNodeAttribute::EType::eSkeleton:
				loadSkeletonNode(pNode, pNode->GetParent()->GetName());
				break;
			}
		}

		nodeList.pop_back();

		const int childNum = pNode->GetChildCount();
		for (int i = 0; i < childNum; ++i)
		{
			nodeList.push_back(pNode->GetChild(i));
		}
	}
}

void MFBXLoader::parseMeshNode(FbxNode *pNode, const uint32 meshIndex)
{
	FBXMesh = pNode->GetMesh();
	_meshList.push_back(FBXMesh);

	int polygonCount = FBXMesh->GetPolygonCount();
	int vertexCounter = 0;								// 맵핑 모드가 eByPolygonVertex일 경우 사용함
	int vertexCount = polygonCount * 3;
    
    VertexList& MeshVertices = _verticesList[meshIndex];
    IndexList& MeshIndices = _indicesList[meshIndex];

    //MeshVertices.resize(FBXMesh->GetControlPointsCount());
    //MeshIndices.resize(polygonCount * 3);

	TotalVertexNum += vertexCount;

	linkMaterial(pNode);

    // 버텍스 키 - 인덱스 쌍
    std::unordered_map<FFBXVertexKey, int> Loaded;

	for (int i = 0; i < polygonCount; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
            int vertexIndex = 3 * i + j;
			int controlPointIndex = FBXMesh->GetPolygonVertex(i, j);
            
            // 한 컨트롤 포인트에는 여러 정점이 있을 수 있고...
            // 그 중에는 UV, NORMAL 등이 다른 경우가 있으니, 다른 점으로 나눠야 함.
            Vertex NewVertex;
            FFBXVertexKey VertexKey;
            VertexKey.ControlPointIndex = controlPointIndex;
			loadPosition(NewVertex, controlPointIndex);
			loadUV(NewVertex, controlPointIndex, vertexIndex, VertexKey);
            LoadColor(NewVertex, controlPointIndex, vertexIndex, VertexKey);
			loadNormal(NewVertex, controlPointIndex, vertexIndex, VertexKey);
			loadTangent(NewVertex, controlPointIndex, vertexIndex, VertexKey);
			loadBinormal(NewVertex, controlPointIndex, vertexIndex, VertexKey);

            // 등록안된 정점이면 정점으로 추가해줌
            if (Loaded.find(VertexKey) == Loaded.end())
            {
                Loaded[VertexKey] = vertexCounter;
                MeshVertices.push_back(NewVertex);
                ControlPointToVertexIndices[meshIndex][controlPointIndex].push_back(vertexCounter);
                ++vertexCounter;
            }

            // 정점의 인덱스를 설정해 줌.
            MeshIndices.push_back(Loaded[VertexKey]);
		}
	}

    FbxAMatrix MeshGlobalTransform = pNode->EvaluateGlobalTransform();

    FbxAMatrix geometryTransform = {
        pNode->GetGeometricTranslation(FbxNode::EPivotSet::eSourcePivot),
        pNode->GetGeometricRotation(FbxNode::EPivotSet::eSourcePivot),
        pNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot)
    };

    FbxAMatrix Temp = (MeshGlobalTransform * geometryTransform).Inverse();
    for (uint32 i = 0; i < 16; ++i)
    {
        uint32 Row = i / 4;
        uint32 Col = i % 4;
        MeshInvGlobalTransforms[meshIndex].m[Row][Col] = static_cast<float>(Temp.mData[Row][Col]);
    }

    std::string Msg = std::string(pNode->GetName()) + ", MeshIndex: " + std::to_string(meshIndex);
    LOG(StringToWString(Msg));
}

void MFBXLoader::linkMaterial(FbxNode *pNode)
{
	// 메시 매터리얼을 씬 매티리얼에 찾아서, 인덱스를 저장
	int MaterialNum = pNode->GetMaterialCount();
	for (int Index = 0; Index < MaterialNum; ++Index)
	{
		FbxSurfaceMaterial* MeshMaterial = pNode->GetMaterial(Index);
		if (nullptr == MeshMaterial)
		{
			continue;
		}

        int SceneMaterialNum = _pScene->GetMaterialCount();
		for (int i = 0; i < SceneMaterialNum; ++i)
		{
			if (MeshMaterial == _pScene->GetMaterial(static_cast<int>(i)))
			{
				MaterialIndices.push_back(i);
				return;
			}
		}
	}

	// 매터리얼이 없으면 기본 값으로 0을 넣음
	if (MaterialNum == 0)
	{
		MaterialIndices.push_back(0);
	}
}

void MFBXLoader::loadPosition(Vertex &vertex, const int controlPointIndex)
{
	vertex.Pos.x = static_cast<float>(FBXMesh->GetControlPointAt(controlPointIndex).mData[0]);
	vertex.Pos.y = static_cast<float>(FBXMesh->GetControlPointAt(controlPointIndex).mData[1]);
	vertex.Pos.z = static_cast<float>(FBXMesh->GetControlPointAt(controlPointIndex).mData[2]);

	// Min 위치 갱신
	MinPosition.x = std::min(MinPosition.x, vertex.Pos.x);
	MinPosition.y = std::min(MinPosition.y, vertex.Pos.y);
	MinPosition.z = std::min(MinPosition.z, vertex.Pos.z);
	// Max 위치 갱신
	MaxPosition.x = std::max(MaxPosition.x, vertex.Pos.x);
	MaxPosition.y = std::max(MaxPosition.y, vertex.Pos.y);
	MaxPosition.z = std::max(MaxPosition.z, vertex.Pos.z);
}

void MFBXLoader::loadUV(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FFBXVertexKey& VertexKey)
{
	FbxGeometryElementUV *uv = FBXMesh->GetElementUV(0);

    int ArrayIndex = 0;
    switch (uv->GetMappingMode())
    {
    case FbxLayerElement::EMappingMode::eByControlPoint:    // 컨트롤 포인트 것을 사용
        ArrayIndex = controlPointIndex;
        break;

    case FbxLayerElement::EMappingMode::eByPolygonVertex:   // 폴리곤의 버텍스 인덱스를 이용
        ArrayIndex = vertexCounter;
        break;

    default:
        DEV_ASSERT_MSG("지원하지 않는 UV MappingMode");
        return;
    }

    switch (uv->GetReferenceMode())
    {
        case FbxLayerElement::EReferenceMode::eDirect:
        {
            VertexKey.UVIndex = ArrayIndex;
            vertex.Tex0.x = static_cast<float>(uv->GetDirectArray().GetAt(ArrayIndex).mData[0]);
            vertex.Tex0.y = 1.f - static_cast<float>(uv->GetDirectArray().GetAt(ArrayIndex).mData[1]);
        }
        break;
        case FbxLayerElement::EReferenceMode::eIndex:
        {

        }
        break;
        case FbxLayerElement::EReferenceMode::eIndexToDirect:
        {
            int index = uv->GetIndexArray().GetAt(ArrayIndex);
            VertexKey.UVIndex = index;
            vertex.Tex0.x = static_cast<float>(uv->GetDirectArray().GetAt(index).mData[0]);
            vertex.Tex0.y = 1.f - static_cast<float>(uv->GetDirectArray().GetAt(index).mData[1]);
        }
        break;
        default:
        {
            DEV_ASSERT_MSG("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint");
        }
        break;
    }
}

void MFBXLoader::LoadColor(Vertex& vertex, const int controlPointIndex, const int vertexCounter, FFBXVertexKey& VertexKey)
{
    FbxGeometryElementVertexColor* Element = FBXMesh->GetElementVertexColor();
    if (Element == nullptr)
    {
        return;
    }

    int ArrayIndex = 0;
    switch (Element->GetMappingMode())
    {
    case FbxLayerElement::EMappingMode::eByControlPoint:    // 컨트롤 포인트 것을 사용
        ArrayIndex = controlPointIndex;
        break;

    case FbxLayerElement::EMappingMode::eByPolygonVertex:   // 폴리곤의 버텍스 인덱스를 이용
        ArrayIndex = vertexCounter;
        break;

    default:
        DEV_ASSERT_MSG("지원하지 않는 UV MappingMode");
        return;
    }

    switch (Element->GetReferenceMode())
    {
    case FbxLayerElement::EReferenceMode::eDirect:
    {
        vertex.Color.x = static_cast<float>(Element->GetDirectArray().GetAt(ArrayIndex).mRed);
        vertex.Color.y = static_cast<float>(Element->GetDirectArray().GetAt(ArrayIndex).mGreen);
        vertex.Color.z = static_cast<float>(Element->GetDirectArray().GetAt(ArrayIndex).mBlue);
    }
    break;
    case FbxLayerElement::EReferenceMode::eIndex:
    {

    }
    break;
    case FbxLayerElement::EReferenceMode::eIndexToDirect:
    {
        int index = Element->GetIndexArray().GetAt(ArrayIndex);
        vertex.Color.x = static_cast<float>(Element->GetDirectArray().GetAt(index).mRed);
        vertex.Color.y = static_cast<float>(Element->GetDirectArray().GetAt(index).mGreen);
        vertex.Color.z = static_cast<float>(Element->GetDirectArray().GetAt(index).mBlue);
    }
    break;
    default:
    {
        DEV_ASSERT_MSG("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint");
    }
    break;
    }
}

void MFBXLoader::loadNormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FFBXVertexKey& VertexKey)
{
	FbxGeometryElementNormal *element = FBXMesh->GetElementNormal(0);

    int ArrayIndex = 0;
    switch (element->GetMappingMode())
    {
    case FbxLayerElement::EMappingMode::eByControlPoint:    // 컨트롤 포인트 것을 사용
        ArrayIndex = controlPointIndex;
        break;

    case FbxLayerElement::EMappingMode::eByPolygonVertex:   // 폴리곤의 버텍스 인덱스를 이용
        ArrayIndex = vertexCounter;
        break;

    default:
        DEV_ASSERT_MSG("지원하지 않는 UV MappingMode");
        return;
    }

    switch (element->GetReferenceMode())
    {
        case FbxLayerElement::EReferenceMode::eDirect:
        {
            //VertexKey.NormalIndex = ArrayIndex;
            vertex.Normal.x = static_cast<float>(element->GetDirectArray().GetAt(ArrayIndex).mData[0]);
            vertex.Normal.y = static_cast<float>(element->GetDirectArray().GetAt(ArrayIndex).mData[1]);
            vertex.Normal.z = static_cast<float>(element->GetDirectArray().GetAt(ArrayIndex).mData[2]);
        }
        break;
        case FbxLayerElement::EReferenceMode::eIndex:
        {

        }
        break;
        case FbxLayerElement::EReferenceMode::eIndexToDirect:
        {
            int index = element->GetIndexArray().GetAt(ArrayIndex);
            //VertexKey.NormalIndex = index;
            vertex.Normal.x = static_cast<float>(element->GetDirectArray().GetAt(index).mData[0]);
            vertex.Normal.y = static_cast<float>(element->GetDirectArray().GetAt(index).mData[1]);
            vertex.Normal.z = static_cast<float>(element->GetDirectArray().GetAt(index).mData[2]);
        }
        break;
        default:
        {
            DEV_ASSERT_MSG("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint");
        }
        break;
    }
}

void MFBXLoader::loadTangent(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FFBXVertexKey& VertexKey)
{
	FbxGeometryElementTangent *element = FBXMesh->GetElementTangent(0);

    if (element == nullptr)
    {
        return;
    }

    int ArrayIndex = 0;
    switch (element->GetMappingMode())
    {
        case FbxLayerElement::EMappingMode::eByControlPoint:    // 컨트롤 포인트 것을 사용
            ArrayIndex = controlPointIndex;
        break;
        case FbxLayerElement::EMappingMode::eByPolygonVertex:   // 폴리곤의 버텍스 인덱스를 이용
            ArrayIndex = vertexCounter;
         break;
        default:
            DEV_ASSERT_MSG("지원하지 않는 UV MappingMode");
         return;
    }

    switch (element->GetReferenceMode())
    {
        case FbxLayerElement::EReferenceMode::eDirect:
        {
            //VertexKey.TangentIndex = ArrayIndex;
            vertex.Tangent.x = static_cast<float>(element->GetDirectArray().GetAt(ArrayIndex).mData[0]);
            vertex.Tangent.y = static_cast<float>(element->GetDirectArray().GetAt(ArrayIndex).mData[1]);
            vertex.Tangent.z = static_cast<float>(element->GetDirectArray().GetAt(ArrayIndex).mData[2]);
        }
        break;
        case FbxLayerElement::EReferenceMode::eIndex:
        {

        }
        break;
        case FbxLayerElement::EReferenceMode::eIndexToDirect:
        {
            int index = element->GetIndexArray().GetAt(ArrayIndex);
            //VertexKey.TangentIndex = index;
            vertex.Tangent.x = static_cast<float>(element->GetDirectArray().GetAt(index).mData[0]);
            vertex.Tangent.y = static_cast<float>(element->GetDirectArray().GetAt(index).mData[1]);
            vertex.Tangent.z = static_cast<float>(element->GetDirectArray().GetAt(index).mData[2]);
        }
        break;
        default:
        {
            DEV_ASSERT_MSG("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint");
        }
        break;
    }
}

void MFBXLoader::loadBinormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FFBXVertexKey& VertexKey)
{
	FbxGeometryElementBinormal *element = FBXMesh->GetElementBinormal(0);

    if (element == nullptr)
    {
        return;
    }

    int ArrayIndex = 0;
    switch (element->GetMappingMode())
    {
    case FbxLayerElement::EMappingMode::eByControlPoint:    // 컨트롤 포인트 것을 사용
        ArrayIndex = controlPointIndex;
        break;
    case FbxLayerElement::EMappingMode::eByPolygonVertex:   // 폴리곤의 버텍스 인덱스를 이용
        ArrayIndex = vertexCounter;
        break;
    default:
        DEV_ASSERT_MSG("지원하지 않는 UV MappingMode");
        return;
    }

    switch (element->GetReferenceMode())
    {
        case FbxLayerElement::EReferenceMode::eDirect:
        {
            //VertexKey.BiNormalIndex = ArrayIndex;
            vertex.Binormal.x = static_cast<float>(element->GetDirectArray().GetAt(ArrayIndex).mData[0]);
            vertex.Binormal.y = static_cast<float>(element->GetDirectArray().GetAt(ArrayIndex).mData[1]);
            vertex.Binormal.z = static_cast<float>(element->GetDirectArray().GetAt(ArrayIndex).mData[2]);
        }
        break;
        case FbxLayerElement::EReferenceMode::eIndex:
        {

        }
        break;
        case FbxLayerElement::EReferenceMode::eIndexToDirect:
        {
            int index = element->GetIndexArray().GetAt(ArrayIndex);
            //VertexKey.BiNormalIndex = index;
            vertex.Binormal.x = static_cast<float>(element->GetDirectArray().GetAt(index).mData[0]);
            vertex.Binormal.y = static_cast<float>(element->GetDirectArray().GetAt(index).mData[1]);
            vertex.Binormal.z = static_cast<float>(element->GetDirectArray().GetAt(index).mData[2]);
        }
        break;
        default:
        {
            DEV_ASSERT_MSG("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint");
        }
        break;
    }
}

void MFBXLoader::loadSkeletonNode(fbxsdk::FbxNode *pNode, const char* parentName)
{
    std::string Name = pNode->GetName();

    uint32 JointIndex = GetSize(Joints);
	NameToJointIndex.emplace(Name, JointIndex);

	FJoint NewJoint;
    NewJoint.Name = Name;
	if(NameToJointIndex.find(parentName) != NameToJointIndex.end())
	{
        NewJoint._parentIndex = NameToJointIndex[parentName];
	}

    FbxAMatrix& GlobalTransform = pNode->EvaluateGlobalTransform();
    auto& Scale = GlobalTransform.GetS();
    auto& Rot = GlobalTransform.GetR();
    auto& Trans = GlobalTransform.GetT();


    int PoseNum = _pScene->GetPoseCount();
    for (int i = 0; i < PoseNum; ++i)
    {
        if (FbxPose* Pose = _pScene->GetPose(i))
        {
            int NodeIndex = Pose->Find(pNode);
            if (NodeIndex != -1)
            {
                FbxMatrix PoseMat = Pose->GetMatrix(NodeIndex); // bind-pose transform (world/model)
                FbxAMatrix APoseMat;

                memcpy((double*)APoseMat, (double*)PoseMat, sizeof(PoseMat.mData));
                Trans = APoseMat.GetT();
                Rot = APoseMat.GetR();
                Scale = APoseMat.GetS();
            }
        }
    }

    NewJoint.Scale = { (float)Scale[0], (float)Scale[1], (float)Scale[2] };
    NewJoint.Rotation = { ToRadian((float)Rot[0]), ToRadian((float)Rot[1]), ToRadian((float)Rot[2]) };
    NewJoint.Position = { (float)Trans[0], (float)Trans[1], (float)Trans[2] };

    Joints.push_back(NewJoint);
    JointNodes.push_back(pNode);
}

void MFBXLoader::loadTexture()
{
	for (uint32 MaterialIndex = 0; MaterialIndex < MaterialNum; ++MaterialIndex)
	{
		FbxSurfaceMaterial* SurfaceMaterial = _pScene->GetMaterial(static_cast<int>(MaterialIndex));
		if (nullptr == SurfaceMaterial)
		{
			continue;
		}
		MaterialTextures.push_back(TextureList(EnumToIndex(ETextureType::End), nullptr));
		LoadTexturesFromFBXMaterial(SurfaceMaterial, MaterialIndex);
	}
}

void MFBXLoader::LoadTexturesFromFBXMaterial(FbxSurfaceMaterial* SurfaceMaterial, uint32 MaterialIndex)
{
	uint32 TextureTypeNum = EnumToIndex(ETextureType::End);
	for (uint32 TextureTypeIndex = 0; TextureTypeIndex < TextureTypeNum; ++TextureTypeIndex)
	{
		// 텍스쳐 타입에 해당하는 프로퍼티를 찾음
		ETextureType TextureType = CastValue<ETextureType>(TextureTypeIndex);
		FbxProperty& Property = SurfaceMaterial->FindProperty(GetTexturePropertyString(TextureType));

		// 텍스쳐를 불러옴
		int TextureNum = Property.GetSrcObjectCount<FbxTexture>();
		//std::wstring DebugString = TEXT("TextureNum: ") + std::to_wstring(TextureNum);
		//LOG(DebugString);
		for (int TextureIndex = 0; TextureIndex < TextureNum; ++TextureIndex)
		{
			FbxFileTexture* FileTexture = FbxCast<FbxFileTexture>(Property.GetSrcObject<FbxTexture>(TextureIndex));

			const char* TempFilePath = FileTexture->GetFileName();
			std::wstring FilePath = Directory + std::filesystem::path(TempFilePath).filename().wstring();
	
			std::shared_ptr<MTexture> Texture = nullptr;
			if (g_ResourceManager->Load(FilePath, Texture))
			{
				TextureList& textureList = MaterialTextures.back();
				textureList[TextureTypeIndex] = Texture;
			}
		}

		int layeredTextureCount = Property.GetSrcObjectCount<FbxLayeredTexture>();
		if (layeredTextureCount > 0)
		{
		}
	}
}

const char* MFBXLoader::GetTexturePropertyString(ETextureType TextureType)
{	
	switch (TextureType)
	{
	case ETextureType::Diffuse:
		return FbxSurfaceMaterial::sDiffuse;
	case ETextureType::Normal:
		return FbxSurfaceMaterial::sBump;
	case ETextureType::Specular:
		return FbxSurfaceMaterial::sSpecular;
	default:
		return nullptr;
	}
}

inline DirectX::XMMATRIX ToXMMatrix(const FbxAMatrix& pSrc)
{
    FbxVector4 S = pSrc.GetS();
    FbxQuaternion Q = pSrc.GetQ();
    FbxVector4 T = pSrc.GetT();

    FbxVector4 R = pSrc.GetR();

    Vec3 Angles = {};
    DXQuaternionToEuler(Vec4(static_cast<float>(Q[0]), static_cast<float>(Q[1]), static_cast<float>(Q[2]), static_cast<float>(Q[3])), Angles.x, Angles.y, Angles.z);

    XMMATRIX ScaleMat = XMMatrixScaling(static_cast<float>(S[0]), static_cast<float>(S[1]), static_cast<float>(S[2]));
    XMMATRIX RotMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Angles));
    XMMATRIX TransMat = XMMatrixTranslation(static_cast<float>(T[0]), static_cast<float>(T[1]), static_cast<float>(T[2]));

    return ScaleMat * RotMat * TransMat;
}

// 조인트 그리기 용
//size_t jointCount = Joints.size();
//std::set<int>jointPositionSet;
//FbxAMatrix temp = geometryTransform.Inverse() * transformMatrix.Inverse() * transformLinkMatrix;
//_jointList[JointIndex]._position = { static_cast<float>(temp[3][0]), static_cast<float>(temp[3][1]), static_cast<float>(temp[3][2]) };
//jointPositionSet.insert(JointIndex);
//for (uint32 i = 0; i < jointCount; ++i)
//{
//    if (jointPositionSet.find(i) == jointPositionSet.end())
//    {
//        int32 parentIndex = Joints[i]._parentIndex;
//        if (parentIndex != -1)
//        {
//            Joints[i]._position = Joints[Joints[i]._parentIndex]._position;
//        }
//    }
//}