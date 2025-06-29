#include "Include.h"
#include "FBXLoader.h"
#include "Texture.h"
#include "Core/ResourceManager.h"
#include <Shlwapi.h>
#include <stdlib.h>
#include <locale>
#include <codecvt>

#undef min
#undef max

using namespace std;
using namespace fbxsdk;

FbxManager *MFBXLoader::_pFbxManager = nullptr;

MFBXLoader::MFBXLoader()
	: _filePathName{ TEXT("") }
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
	: _filePathName	{ filePathName }
	, Directory		{ filePathName }
	, _pImporter	{ nullptr }
	, _pScene		{ nullptr }
	, _pSkeleton	{ nullptr }
	, _pAnimStack	{ nullptr }
	, GeometryCount{ 0 }
	, MaterialNum{ 0 }
	, meshCounter{ 0 }
{
	LoadMesh(_filePathName);
}

MFBXLoader::~MFBXLoader()
{
	_pScene->Destroy();
	_pImporter->Destroy();
	//_pFbxManager->Destroy();
}

void MFBXLoader::LoadAnim(std::vector<AnimationClip>& animationClipList)
{
	// 애니메이션 관련 처리
	int animStackCount = _pImporter->GetAnimStackCount();
	animationClipList.resize(animStackCount);

	PerformanceTimer timer;
	for (int animStackIndex = 0; animStackIndex < animStackCount; ++animStackIndex)
	{
		_pAnimStack = _pScene->GetCurrentAnimationStack();
		FbxString animStackName = _pAnimStack->GetName();
		FbxTakeInfo* pTakeInfo = _pScene->GetTakeInfo(animStackName);
		FbxTime startTime = pTakeInfo->mLocalTimeSpan.GetStart();
		FbxTime endTime = pTakeInfo->mLocalTimeSpan.GetStop();

		size_t jointCount = _jointList.size();
		animationClipList[animStackIndex]._keyFrameLists.resize(jointCount); // 조인트 마다 키프레임을 저장하기 위해
		animationClipList[animStackIndex]._animationName = animStackName.Buffer();
		animationClipList[animStackIndex]._startFrame = CastValue<uint32>(startTime.GetFrameCount(FbxTime::eFrames30));
		animationClipList[animStackIndex]._endFrame = CastValue<uint32>(endTime.GetFrameCount(FbxTime::eFrames30));
		animationClipList[animStackIndex]._frameCount = CastValue<uint32>((endTime - startTime).GetFrameCount(FbxTime::eFrames30)) + 1;
		animationClipList[animStackIndex]._duration = (endTime - startTime).GetSecondDouble();

		for (auto& frameMatricesPerGeometry : animationClipList[animStackIndex]._keyFrameLists)
		{
			frameMatricesPerGeometry.resize(GeometryCount);
		}

		std::set<int>jointPositionSet;
		for (uint32 meshIndex = 0; meshIndex < GeometryCount; ++meshIndex)
		{
			FbxMesh* pMesh = _meshList[meshIndex];
			FbxNode* pMeshNode = pMesh->GetNode(0);

			FbxAMatrix geometryTransform = { pMeshNode->GetGeometricTranslation(FbxNode::EPivotSet::eSourcePivot),
												pMeshNode->GetGeometricRotation(FbxNode::EPivotSet::eSourcePivot),
												pMeshNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot) };
			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;
			FbxAMatrix globalBindPoseInverseMatrix;

			FbxTime currentTime;
			Mat4 matrix;

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

					// 조인트의 바인드포즈 인버스 매트릭스 얻기
					pCluster->GetTransformMatrix(transformMatrix);
					pCluster->GetTransformLinkMatrix(transformLinkMatrix);
					globalBindPoseInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;
					// VertexAtTimeT = TransformationOfPoseAtTimeT * InverseOfGlobalBindPoseMatrix * VertexAtBindingTime 
					// 에서 InverseOfGlobalBindPoseMatrix를 얻어냈음

					const char* jointName = pCluster->GetLink()->GetName();
					//if (_jointIndexMap.end() == _jointIndexMap.find(jointName))
					//{
					//	DEV_ASSERT_MSG("찾을 수 없는 Bone입니다!")
					//	continue;
					//}
					int jointIndex = _jointIndexMap[jointName];

					// 조인트 그리기 용
					FbxAMatrix temp = geometryTransform.Inverse() * transformMatrix.Inverse() * transformLinkMatrix;
					_jointList[jointIndex]._position = { static_cast<float>(temp[3][0]), static_cast<float>(temp[3][1]), static_cast<float>(temp[3][2]) };
					jointPositionSet.insert(jointIndex);

					double* weights = pCluster->GetControlPointWeights();
					if (weights == nullptr)
					{
						continue;
					}

					XMStoreFloat4x4(&_jointList[jointIndex]._globalBindPoseInverseMatrix, ToXMMatrix(globalBindPoseInverseMatrix));

					//{
					//	PerformanceTimer timer(TEXT("WeigtIndex"));
					int controlPointIndexCount = pCluster->GetControlPointIndicesCount();
					for (int j = 0; j < controlPointIndexCount; ++j)
					{
						int controlPointIndex = pCluster->GetControlPointIndices()[j];

						// 컨트롤 포인트가 공유되는 버텍스들의 인덱스를 가져옴
						std::vector<int>& vertexIndices = _indexMap[meshIndex][controlPointIndex];
						for (int vertexIndex : vertexIndices)
						{
							if (_verticesList[meshIndex][vertexIndex].BlendIndex[0] == 0)
							{
								_verticesList[meshIndex][vertexIndex].BlendIndex[0] = jointIndex;
								_verticesList[meshIndex][vertexIndex].BlendWeight.x = static_cast<float>(weights[j]);
							}
							else if (_verticesList[meshIndex][vertexIndex].BlendIndex[1] == 0)
							{
								_verticesList[meshIndex][vertexIndex].BlendIndex[1] = jointIndex;
								_verticesList[meshIndex][vertexIndex].BlendWeight.y = static_cast<float>(weights[j]);
							}
							else if (_verticesList[meshIndex][vertexIndex].BlendIndex[2] == 0)
							{
								_verticesList[meshIndex][vertexIndex].BlendIndex[2] = jointIndex;
								_verticesList[meshIndex][vertexIndex].BlendWeight.z = static_cast<float>(weights[j]);
							}
							else if (_verticesList[meshIndex][vertexIndex].BlendIndex[3] == 0)
							{
								_verticesList[meshIndex][vertexIndex].BlendIndex[3] = jointIndex;
								_verticesList[meshIndex][vertexIndex].BlendWeight.w = static_cast<float>(weights[j]);
							}
						}
					}
					//}

					//{
					//	PerformanceTimer timer(TEXT("LoadFrameMatrices"));
						// 프레임에 존재하는 키 프레임을 저장해야 함
					if (animationClipList[animStackIndex]._keyFrameLists[jointIndex][meshIndex].capacity() == 0)
					{
						animationClipList[animStackIndex]._keyFrameLists[jointIndex][meshIndex].reserve(animationClipList[animStackIndex]._frameCount);	// 조인트마다 키프레임 공간 예약. 키 프레임이 최대 프레임 수만큼 가질 수 있음. 굳이? 싶지만 지금은 최적화 생각안하고 구현
					}

					for (uint32 frame = animationClipList[animStackIndex]._startFrame; frame <= animationClipList[animStackIndex]._endFrame; ++frame)
					{
						currentTime.SetFrame(static_cast<FbxLongLong>(frame), FbxTime::eFrames30);

						//FbxAMatrix currentTransformOffset = (pMeshNode->EvaluateGlobalTransform(currentTime) * geometryTransform).Inverse();	// 메시의 글로벌 트랜스폼 * 지오메트리 트랜스폼
						XMStoreFloat4x4(&matrix, ToXMMatrix((pMeshNode->EvaluateGlobalTransform(currentTime) * geometryTransform).Inverse() * pCluster->GetLink()->EvaluateGlobalTransform(currentTime)));

						animationClipList[animStackIndex]._keyFrameLists[jointIndex][meshIndex].emplace_back(matrix);
					}
					//}

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
					log += jointName;
					log += "/";
					log += std::to_string(jointIndex);
					log += "\r\n";
					OutputDebugStringA(log.c_str());
#endif
				}
			}

			for (uint32 i = 0; i < jointCount; ++i)
			{
				if (jointPositionSet.find(i) == jointPositionSet.end())
				{
					int32 parentIndex = _jointList[i]._parentIndex;
					if (parentIndex != -1)
					{
						_jointList[i]._position = _jointList[_jointList[i]._parentIndex]._position;
					}
				}
			}
		}
	}

	//_jointList[0]._translation = { 0.f, 0.f, 0.f };
}

bool MFBXLoader::LoadMesh(const wstring& Path)
{
	_filePathName = Path;
	Directory = Path.substr(0, Path.find_last_of('/') + 1);

	InitializeFbxSdk();
	convertScene();

	GeometryCount = _pScene->GetGeometryCount();
	_verticesList.resize(GeometryCount);
	_indicesList.resize(GeometryCount);
	_linkList.reserve(GeometryCount);
	_indexMap.resize(GeometryCount);

	MaterialNum = static_cast<uint32>(_pScene->GetMaterialCount());
	//_texturesList.reserve(MaterialNum);
	//for (auto& tl : _texturesList)
	//{
	//	tl.resize(CastValue<uint32>(ETextureType::End), nullptr);
	//}
	_texturesList.reserve(MaterialNum);

	loadNode();
	loadTexture();

	return false;
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
	WStringToString(_filePathName, FilePathName, 255);
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
	return CastValue<uint32>(_jointList.size());
}

const uint32 MFBXLoader::GetGeometryNum() const
{
	return GeometryCount;
}

const uint32 MFBXLoader::GetMaterialNum() const
{
	return MaterialNum;
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
	return _texturesList;
}

const std::vector<uint32>& MFBXLoader::getLinkList() const
{
	return _linkList;
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
                _indexMap[meshIndex][controlPointIndex].push_back(vertexCounter);
                ++vertexCounter;
            }

            // 정점의 인덱스를 설정해 줌.
            MeshIndices.push_back(Loaded[VertexKey]);
		}
	}
}

void MFBXLoader::linkMaterial(FbxNode *pNode)
{
	// 하나의 매터리얼을 여러 메쉬가 사용하는 경우를 위해 링크
	int NodeMaterialNum = pNode->GetMaterialCount();
	for (int Index = 0; Index < NodeMaterialNum; ++Index)
	{
		FbxSurfaceMaterial* SurfaceMaterial = pNode->GetMaterial(Index);
		if (nullptr == SurfaceMaterial)
		{
			continue;
		}

		// 실제로 사용하고 있는 매터리얼Index를 찾음
		for (uint32 i = 0; i < MaterialNum; ++i)
		{
			if (SurfaceMaterial == _pScene->GetMaterial(static_cast<int>(i)))
			{
				_linkList.push_back(i);
				return;
			}
		}
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
	_jointIndexMap.emplace(pNode->GetName(), static_cast<uint32>(_jointList.size()));

	FJoint joint;
	joint._name	= pNode->GetName();
	if(_jointIndexMap.find(parentName) != _jointIndexMap.end())
	{
		joint._parentIndex	= _jointIndexMap[parentName];
	}
	;

	auto& trans = pNode->GeometricTranslation.Get();
	joint._position = { (float)trans[0], (float)trans[1], (float)trans[2] };

	_jointList.push_back(joint);
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
		_texturesList.push_back(TextureList(EnumToIndex(ETextureType::End), nullptr));
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
				TextureList& textureList = _texturesList.back();
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
