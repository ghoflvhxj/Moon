﻿#include "FBXLoader.h"
#include "FBXSDK/fbxsdk/scene/fbxaxissystem.h"

#include "Texture.h"
#include "Core/ResourceManager.h"
#include "Core/Serialize/JsonSerializer.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Material.h"

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
	LoadMesh(Path);
}

MFBXLoader::~MFBXLoader()
{
	_pScene->Destroy();
	_pImporter->Destroy();
	//_pFbxManager->Destroy();
}

void MFBXLoader::SafeDestroy(fbxsdk::FbxObject*& InObject)
{
    if (InObject)
    {
        InObject->Destroy();
        InObject = nullptr;
    }
}

void MFBXLoader::LoadAnim(std::vector<AnimationClip>& OutAnimationClips)
{
    // 메시를 그릴거임.
    // 근데 애니메이션이 적용됬다면, 정점들에 애니메이션 행렬을 곱해야 함.
    // 점에 영향을 주는 조인트 4개가 있는데 이거를 같이 보냄. (인덱스, 수치 정보)
    // 그래서 점에 조인트 행렬과 수치를 곱해줌

	int AnimStackCount = _pImporter->GetAnimStackCount();
	OutAnimationClips.resize(AnimStackCount);

	PerformanceTimer timer;
	for (int AnimStackIndex = 0; AnimStackIndex < AnimStackCount; ++AnimStackIndex)
	{
		_pAnimStack = _pScene->GetCurrentAnimationStack();
		FbxString animStackName = _pAnimStack->GetName();
		FbxTakeInfo* pTakeInfo = _pScene->GetTakeInfo(animStackName);

        AnimationClip& CurrentAnimClip = OutAnimationClips[AnimStackIndex];
		CurrentAnimClip.Name = animStackName.Buffer();
        CurrentAnimClip.SetFrameInfo(pTakeInfo->mLocalTimeSpan.GetStart(), pTakeInfo->mLocalTimeSpan.GetStop());

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

        // 조인트를 얻기 위해 메시->디포머->스킨->클러스터 순으로 파고듬
		for (uint32 meshIndex = 0; meshIndex < GeometryCount; ++meshIndex)
		{
			FbxMesh* pMesh = _meshList[meshIndex];
			FbxNode* pMeshNode = pMesh->GetNode(0);

			FbxAMatrix geometryTransform = { 
                pMeshNode->GetGeometricTranslation(FbxNode::EPivotSet::eSourcePivot),
			    pMeshNode->GetGeometricRotation(FbxNode::EPivotSet::eSourcePivot),
				pMeshNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot) 
            };

			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;
			FbxAMatrix globalBindPoseInverseMatrix;

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
                    int JointIndex = JointIndices[JointName];

					// 조인트의 바인드포즈 인버스 매트릭스 얻기
					pCluster->GetTransformMatrix(transformMatrix);
					pCluster->GetTransformLinkMatrix(transformLinkMatrix);
					globalBindPoseInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;
                    XMStoreFloat4x4(&Joints[JointIndex]._globalBindPoseInverseMatrix, ToXMMatrix(globalBindPoseInverseMatrix));

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
                                    if (_verticesList[meshIndex][VertexIndex].BlendIndex[BlendCounter] == 0)
                                    {
                                        _verticesList[meshIndex][VertexIndex].BlendIndex[BlendCounter] = JointIndex;
                                        _verticesList[meshIndex][VertexIndex].BlendWeight[BlendCounter] = Weight;
                                        break;
                                    }
                                }
                            }
                        }
					}

                    // 조인트가 프레임에 메시에 영향을 준다면 저장해야 함. ex) 10프레임에 조인트A가 메시0, 메시1에 영향을 준다
                    for (uint32 Frame = 0; Frame < CurrentAnimClip.TotalFrame; ++Frame)
                    {
                        FbxTime currentTime;
                        currentTime.SetFrame(static_cast<FbxLongLong>(CurrentAnimClip.StartFrame + Frame), FbxTime::eFrames24);

                        //FbxAMatrix currentTransformOffset = (pMeshNode->EvaluateGlobalTransform(currentTime) * geometryTransform).Inverse();	// 메시의 글로벌 트랜스폼 * 지오메트리 트랜스폼
                        FbxAMatrix& Test = (pMeshNode->EvaluateGlobalTransform(currentTime) * geometryTransform).Inverse() * pCluster->GetLink()->EvaluateGlobalTransform(currentTime);
                        XMStoreFloat4x4(&CurrentAnimClip.GetKeyFrame(Frame).GetJointMatrix(JointIndex), ToXMMatrix(Test));
                    }

#ifdef _DEBUG
					std::string log;
					log += "MeshName: ";
					log += pMeshNode->GetName();
					log += ", MeshIndex: ";
					log += std::to_string(meshIndex);
					log += ", DeformerCount/Index: ";
					log += std::to_string(deformerCount);
					log += "/";
					log += std::to_string(deformerIndex);
					log += ", ClusterCount/Index: ";
					log += std::to_string(clusterCount);
					log += "/";
					log += std::to_string(clusterIndex);
					log += ", jointName/Index: ";
					log += JointName;
					log += "/";
					//log += std::to_string(jointIndex);
					log += "\r\n";
					OutputDebugStringA(log.c_str());
#endif
				}
			}
		}
	}

	//_jointList[0]._translation = { 0.f, 0.f, 0.f };
}

bool MFBXLoader::LoadMesh(const wstring& InPath)
{
	Path = InPath;
	Directory = Path.substr(0, Path.find_last_of('/') + 1);
    std::filesystem::path PathObject(Path);
    Name = PathObject.filename().wstring();
    Name = Name.substr(0, Name.find_last_of('.'));
    Extension = PathObject.extension();

	InitializeFbxSdk();
	convertScene();

	GeometryCount = _pScene->GetGeometryCount();
	_verticesList.resize(GeometryCount);
	_indicesList.resize(GeometryCount);
	MaterialIndices.reserve(GeometryCount);
	ControlPointToVertexIndices.resize(GeometryCount);

	MaterialNum = static_cast<uint32>(_pScene->GetMaterialCount());
	//_texturesList.reserve(MaterialNum);
	//for (auto& tl : _texturesList)
	//{
	//	tl.resize(CastValue<uint32>(ETextureType::End), nullptr);
	//}
	MaterialTextures.reserve(MaterialNum);

	loadNode();
	loadTexture();

	return false;
}

void MFBXLoader::SaveJsonAsset(const std::wstring& InPath)
{
    Path = InPath;
    Directory = Path.substr(0, Path.find_last_of('/') + 1);
    std::filesystem::path PathObject(Path);
    Name = PathObject.filename().wstring();
    Name = Name.substr(0, Name.find_last_of('.'));
    Extension = PathObject.extension();

    InitializeFbxSdk();

    // StaticMesh를 FBX를 통해 생성 후 Json 저장
    std::shared_ptr<StaticMesh> NewStaticMesh = std::make_shared<StaticMesh>();
    NewStaticMesh->LoadFromFBX(InPath, *this);

    std::set<uint32> UniqueMaterialIndices;
    for (uint32 MaterialIndex : MaterialIndices)
    {
        UniqueMaterialIndices.emplace(MaterialIndex);
    }

    // 매터리얼 생성 후 저장
    uint32 MaterialNum = GetSize(UniqueMaterialIndices);
    std::vector<std::shared_ptr<MMaterial>> Materials(MaterialNum, std::make_shared<MMaterial>());
    for (uint32 MaterialIndex = 0; MaterialIndex < MaterialNum; ++MaterialIndex)
    {
        std::shared_ptr<MMaterial> NewMaterial = std::make_shared<MMaterial>();

        if (MaterialIndex < GetSize(MaterialTextures))
        {
            NewMaterial->setTextures(MaterialTextures[MaterialIndex]);
        }

        NewMaterial->SetName(GetMaterialIName(MaterialIndex));
        NewMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso"));

        std::wstring MatPath = Directory + GetMaterialIName(MaterialIndex) + TEXT(".json");

        MJsonSerializer MatSerializer;
        MatSerializer.Serialize(*NewMaterial, MatPath, true);

        // 메시에 매터리얼 경로를 넣어줌
        NewStaticMesh->MaterialPaths.push_back(MatPath);
    }

    // 이제 매터리얼 정보가 채워진 StaticMesh 저장할 수 있음.
    MJsonSerializer Serializer;
    std::wstring MeshPath = Directory + Name + TEXT(".json");
    Serializer.Serialize(*NewStaticMesh, MeshPath, true);
}

void MFBXLoader::InitializeFbxSdk()
{
	if (nullptr == _pFbxManager)
		initializeSDK();

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
}

void MFBXLoader::convertScene()
{
	FbxAxisSystem directXAxisSys(FbxAxisSystem::EPreDefinedAxisSystem::eDirectX);
	directXAxisSys.ConvertScene(_pScene);

	FbxGeometryConverter geometryConverter(_pFbxManager);
	geometryConverter.Triangulate(_pScene, true);
}

void MFBXLoader::initializeSDK()
{
	_pFbxManager = FbxManager::Create();

	FbxIOSettings *ios = FbxIOSettings::Create(_pFbxManager, IOSROOT);
	_pFbxManager->SetIOSettings(ios);
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
            const char* Name = Material->GetName();
            std::wstring WName;
            StringToWString(Name, WName);
            return WName;
        }
    }

    return TEXT("");
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
			switch (pNodeAttribute->GetAttributeType())
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
    std::unordered_map<FVertexKey, int> Loaded;

	for (int i = 0; i < polygonCount; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
            int vertexIndex = 3 * i + j;
			int controlPointIndex = FBXMesh->GetPolygonVertex(i, j);
            
            // 한 컨트롤 포인트에는 여러 정점이 있을 수 있고...
            // 그 중에는 UV, NORMAL 등이 다른 경우가 있으니, 다른 점으로 나눠야 함.
            Vertex NewVertex;
            FVertexKey VertexKey;
            VertexKey.ControlPointIndex = controlPointIndex;
			loadPosition(NewVertex, controlPointIndex);
			loadUV(NewVertex, controlPointIndex, vertexIndex, VertexKey);
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
		for (uint32 i = 0; i < SceneMaterialNum; ++i)
		{
			if (MeshMaterial == _pScene->GetMaterial(static_cast<int>(i)))
			{
				MaterialIndices.push_back(i);
				return;
			}
		}
	}

    // 매터리얼이 없으면 기본으로 하나 추가
    if (MaterialIndices.empty())
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

void MFBXLoader::loadUV(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FVertexKey& VertexKey)
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

void MFBXLoader::loadNormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FVertexKey& VertexKey)
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

void MFBXLoader::loadTangent(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FVertexKey& VertexKey)
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

void MFBXLoader::loadBinormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FVertexKey& VertexKey)
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

void MFBXLoader::loadAnimation()
{

}

void MFBXLoader::loadSkeletonNode(fbxsdk::FbxNode *pNode, const char* parentName)
{
    uint32 JointIndex = GetSize(Joints);
	JointIndices.emplace(pNode->GetName(), JointIndex);

	FJoint NewJoint;
	if(JointIndices.find(parentName) != JointIndices.end())
	{
        NewJoint._parentIndex = JointIndices[parentName];
	}

	auto& trans = pNode->GeometricTranslation.Get();
    NewJoint._position = { (float)trans[0], (float)trans[1], (float)trans[2] };

	Joints.push_back(NewJoint);
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
		std::wstring DebugString = TEXT("TextureNum: ") + std::to_wstring(TextureNum);
		OutputDebugString(DebugString.c_str());
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

	//int layeredTextureCount = Property.GetSrcObjectCount<FbxLayeredTexture>();
	//if (0 < layeredTextureCount)
	//{
	//	for (int j = 0; j < layeredTextureCount; ++j)
	//	{
	//		FbxLayeredTexture *layeredTexture = FbxCast<FbxLayeredTexture>(Property.GetSrcObject<FbxLayeredTexture>(j));
	//		int textureCount = layeredTexture->GetSrcObjectCount<FbxTexture>();
	//		for (int k = 0; k < textureCount; ++k)
	//		{
	//			FbxTexture *texture = layeredTexture->GetSrcObject<FbxTexture>(k);
	//			FbxFileTexture *fileTexture = FbxCast<FbxFileTexture>(texture);

	//			const char *a = fileTexture->GetFileName();
	//			int b = 0;
	//		}
	//	}
	//}
	//else
	//{
	//	int TextureNum = Property.GetSrcObjectCount<FbxTexture>();
	//	for (int TextureIndex = 0; TextureIndex < TextureNum; ++TextureIndex)
	//	{
	//		FbxTexture* FbxTexture = Property.GetSrcObject<FbxTexture>(TextureIndex);
	//		FbxFileTexture *FbxFileTexture = FbxCast<FbxFileTexture>(FbxTexture);

	//		char FilePath[255] = { 0, };
	//		WStringToString(_filePath, FilePath, 255);
	//		strcat_s(FilePath, 255, PathFindFileNameA(FbxFileTexture->GetFileName()));

	//		std::shared_ptr<MTexture> Texture = std::make_shared<MTexture>(FilePath);
	//		
	//		TextureList &textureList = _texturesList.back();
	//		textureList[EnumToIndex(textureType)] = Texture;
	//	}
	//}
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
	return {
		static_cast<FLOAT>(pSrc[0][0]), static_cast<FLOAT>(pSrc[0][1]), static_cast<FLOAT>(pSrc[0][2]), static_cast<FLOAT>(pSrc[0][3]),
		static_cast<FLOAT>(pSrc[1][0]), static_cast<FLOAT>(pSrc[1][1]), static_cast<FLOAT>(pSrc[1][2]), static_cast<FLOAT>(pSrc[1][3]),
		static_cast<FLOAT>(pSrc[2][0]), static_cast<FLOAT>(pSrc[2][1]), static_cast<FLOAT>(pSrc[2][2]), static_cast<FLOAT>(pSrc[2][3]),
		static_cast<FLOAT>(pSrc[3][0]), static_cast<FLOAT>(pSrc[3][1]), static_cast<FLOAT>(pSrc[3][2]), static_cast<FLOAT>(pSrc[3][3])
	};
}
