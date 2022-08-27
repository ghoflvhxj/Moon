#include "stdafx.h"
#include "FBXLoader.h"

#include "TextureComponent.h"
#include <Shlwapi.h>

#include <stdlib.h>

using namespace fbxsdk;

FbxManager *FBXLoader::_pFbxManager = nullptr;

FBXLoader::FBXLoader(const char *filePathName)
	: _filePathName	{ filePathName }
	, _filePath		{ filePathName }
	, _pImporter	{ nullptr }
	, _pScene		{ nullptr }
	, _pSkeleton	{ nullptr }
	, _pAnimStack	{ nullptr }
	, _geometryCount{ 0 }
	, _materialCount{ 0 }
	, meshCounter{ 0 }
{
	initializeFbxSdk();
	convertScene();
	loadModel();
}

FBXLoader::FBXLoader(const char *filePathName, std::vector<AnimationClip> &animationClipList)
	: _filePathName{ filePathName }
	, _filePath{ filePathName }
	, _pImporter{ nullptr }
	, _pScene{ nullptr }
	, _pSkeleton{ nullptr }
	, _pAnimStack{ nullptr }
	, _geometryCount{ 0 }
	, _materialCount{ 0 }
	, meshCounter{ 0 }
{
	initializeFbxSdk();
	convertScene();
	loadModel();

	//---------------------------------------------------------------
	// process joint and animation
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
		animationClipList[animStackIndex]._animationName	= animStackName.Buffer();
		animationClipList[animStackIndex]._startFrame		= CastValue<uint32>(startTime.GetFrameCount(FbxTime::eFrames30));
		animationClipList[animStackIndex]._endFrame			= CastValue<uint32>(endTime.GetFrameCount(FbxTime::eFrames30));
		animationClipList[animStackIndex]._frameCount		= CastValue<uint32>((endTime - startTime).GetFrameCount(FbxTime::eFrames30)) + 1;
		animationClipList[animStackIndex]._duration			= (endTime - startTime).GetSecondDouble();

		for (auto &frameMatricesPerGeometry : animationClipList[animStackIndex]._keyFrameLists)
		{
			frameMatricesPerGeometry.resize(_geometryCount);
		}

		std::set<int>jointPositionSet;
		for (uint32 meshIndex = 0; meshIndex < _geometryCount; ++meshIndex)
		{
			FbxMesh *pMesh = _meshList[meshIndex];
			FbxNode *pMeshNode = pMesh->GetNode(0);

			FbxAMatrix geometryTransform = {	pMeshNode->GetGeometricTranslation(FbxNode::EPivotSet::eSourcePivot),
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
				FbxDeformer *pDeformer = pMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin);
				if (nullptr == pDeformer)
				{
					continue;
				}

				FbxSkin *pSkin = reinterpret_cast<FbxSkin*>(pDeformer);

				int clusterCount = pSkin->GetClusterCount();
				for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
				{
					FbxCluster *pCluster = pSkin->GetCluster(clusterIndex);

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
							std::vector<int> &vertexIndices = _indexMap[meshIndex][controlPointIndex];
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

FBXLoader::~FBXLoader()
{
	_pScene->Destroy();
	_pImporter->Destroy();
	//_pFbxManager->Destroy();
}

void FBXLoader::initializeFbxSdk()
{
	_filePath = _filePath.substr(0, _filePath.find_last_of('/') + 1);

	if (nullptr == _pFbxManager)
		initializeSDK();

	_pImporter = FbxImporter::Create(_pFbxManager, "");
	if (nullptr == _pImporter)
	{
		DEV_ASSERT_MSG("FbxImporter가 nullptr 입니다!");
	}

	if (false == _pImporter->Initialize(_filePathName.c_str()))
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

void FBXLoader::convertScene()
{
	FbxAxisSystem directXAxisSys(FbxAxisSystem::EPreDefinedAxisSystem::eDirectX);
	directXAxisSys.ConvertScene(_pScene);

	FbxGeometryConverter geometryConverter(_pFbxManager);
	geometryConverter.Triangulate(_pScene, true);
}

void FBXLoader::loadModel()
{
	_geometryCount = _pScene->GetGeometryCount();
	_verticesList.resize(_geometryCount);
	_indicesList.resize(_geometryCount);
	_linkList.reserve(_geometryCount);
	_indexMap.resize(_geometryCount);

	_materialCount = static_cast<uint32>(_pScene->GetMaterialCount());
	_texturesList.reserve(_materialCount);

	loadNode();
	loadTexture();
}

void FBXLoader::initializeSDK()
{
	_pFbxManager = FbxManager::Create();

	FbxIOSettings *ios = FbxIOSettings::Create(_pFbxManager, IOSROOT);
	_pFbxManager->SetIOSettings(ios);
}

const uint32 FBXLoader::getJointCount() const
{
	return CastValue<uint32>(_jointList.size());
}

const uint32 FBXLoader::getGeometryCount() const
{
	return _geometryCount;
}

const uint32 FBXLoader::getMaterialCount() const
{
	return _materialCount;
}

const uint32 FBXLoader::getVertexCount() const
{
	return _vertexCount;
}

std::vector<VertexList> &FBXLoader::getVerticesList()
{
	return _verticesList;
}

std::vector<IndexList> &FBXLoader::getIndicesList()
{
	return _indicesList;
}

std::vector<TextureList> &FBXLoader::getTextures()
{
	return _texturesList;
}

const std::vector<uint32>& FBXLoader::getLinkList() const
{
	return _linkList;
}

void FBXLoader::loadNode()
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
				parseMeshNode(pNode, meshCounter);
				++meshCounter;
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

void FBXLoader::parseMeshNode(FbxNode *pNode, const uint32 meshIndex)
{
	_pMesh = pNode->GetMesh();
	_meshList.push_back(_pMesh);

	int polygonCount = _pMesh->GetPolygonCount();
	int vertexCounter = 0;							// 맵핑 모드가 eByPolygonVertex일 경우 사용함
	int vertexCount = polygonCount * 3;
	_verticesList[meshIndex].resize(polygonCount * 3);	// 중첩된 버텍스를 허용하지 않기 때문에 인덱스의 수와 같아짐...
	_indicesList[meshIndex].reserve(polygonCount * 3);

	_vertexCount += vertexCount;

	linkMaterial(pNode);

	for (int i = 0; i < polygonCount; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			int controlPointIndex = _pMesh->GetPolygonVertex(i, j);
			int vertexIndex = (3 * i) + j;

			loadPosition(_verticesList[meshIndex][vertexIndex], controlPointIndex);
			loadUV(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
			loadNormal(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
			loadTangent(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
			loadBinormal(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
			_indicesList[meshIndex].push_back((3 * i) + j); // 수정해야 함
			
			_indexMap[meshIndex][controlPointIndex].emplace_back(vertexIndex);

			++vertexCounter;
		}

		//int controlPointIndex = _pMesh->GetPolygonVertex(i, 0);
		//int vertexIndex = (3 * i) + 0;
		//loadPosition(_verticesList[meshIndex][vertexIndex], controlPointIndex);
		//loadUV(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//loadNormal(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//loadTangent(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//loadBinormal(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//_indicesList.back().push_back((3 * i) + 0);

		//++vertexCounter;

		//controlPointIndex = _pMesh->GetPolygonVertex(i, 2);
		//vertexIndex = (3 * i) + 2;
		//loadPosition(_verticesList[meshIndex][vertexIndex], controlPointIndex);
		//loadUV(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//loadNormal(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//loadTangent(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//loadBinormal(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//_indicesList.back().push_back((3 * i) + 2);

		//++vertexCounter;


		//controlPointIndex = _pMesh->GetPolygonVertex(i, 1);
		//vertexIndex = (3 * i) + 1;
		//loadPosition(_verticesList[meshIndex][vertexIndex], controlPointIndex);
		//loadUV(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//loadNormal(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//loadTangent(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//loadBinormal(_verticesList[meshIndex][vertexIndex], controlPointIndex, vertexCounter);
		//_indicesList.back().push_back((3 * i) + 1);

		//++vertexCounter;
	}
}

void FBXLoader::linkMaterial(FbxNode *pNode)
{
	// 하나의 매터리얼을 여러 메쉬가 사용하는 경우를 위해 링크
	int materialCouint = pNode->GetMaterialCount();
	for (int index = 0; index < materialCouint; ++index)
	{
		FbxSurfaceMaterial *pSufaceMaterial = pNode->GetMaterial(index);
		if (nullptr == pSufaceMaterial)
		{
			continue;
		}

		for (uint32 i = 0; i < _materialCount; ++i)
		{
			if (pSufaceMaterial == _pScene->GetMaterial(static_cast<int>(i)))
			{
				_linkList.push_back(i);
				return;
			}
		}
	}
}

void FBXLoader::loadPosition(Vertex &vertex, const int controlPointIndex)
{
	double a = _pMesh->GetControlPointAt(controlPointIndex).mData[0];
	vertex.Pos.x = ToFloat(_pMesh->GetControlPointAt(controlPointIndex).mData[0]);
	vertex.Pos.y = ToFloat(_pMesh->GetControlPointAt(controlPointIndex).mData[1]);
	vertex.Pos.z = ToFloat(_pMesh->GetControlPointAt(controlPointIndex).mData[2]);

	if (_min.x < vertex.Pos.x) _min.x = vertex.Pos.x;
	if (_min.y < vertex.Pos.y) _min.y = vertex.Pos.y;
	if (_min.z < vertex.Pos.z) _min.z = vertex.Pos.z;

	if (_max.x > vertex.Pos.x) _max.x = vertex.Pos.x;
	if (_max.y > vertex.Pos.y) _max.y = vertex.Pos.y;
	if (_max.z > vertex.Pos.z) _max.z = vertex.Pos.z;
}

void FBXLoader::loadUV(Vertex &vertex, const int controlPointIndex, const int vertexCounter)
{
	FbxGeometryElementUV *uv = _pMesh->GetElementUV(0);
	switch (uv->GetMappingMode())
	{
	case FbxLayerElement::EMappingMode::eByControlPoint:
	{
		switch (uv->GetReferenceMode())
		{
		case FbxLayerElement::EReferenceMode::eDirect:
		{
			vertex.Tex0.x = ToFloat(uv->GetDirectArray().GetAt(controlPointIndex).mData[0]);
			vertex.Tex0.y = 1.f - ToFloat(uv->GetDirectArray().GetAt(controlPointIndex).mData[1]);
		}
		break;
		case FbxLayerElement::EReferenceMode::eIndex:
		{

		}
		break;
		case FbxLayerElement::EReferenceMode::eIndexToDirect:
		{
			int index = uv->GetIndexArray().GetAt(controlPointIndex);
			vertex.Tex0.x = ToFloat(uv->GetDirectArray().GetAt(index).mData[0]);
			vertex.Tex0.y = 1.f - ToFloat(uv->GetDirectArray().GetAt(index).mData[1]);
		}
		break;
		default:
		{
			DEV_ASSERT_MSG("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint");
		}
		break;
		}
	}
	break;

	case FbxLayerElement::EMappingMode::eByPolygonVertex:
	{
		switch (uv->GetReferenceMode())
		{
		case FbxLayerElement::EReferenceMode::eDirect:
		{
			vertex.Tex0.x = ToFloat(uv->GetDirectArray().GetAt(vertexCounter).mData[0]);
			vertex.Tex0.y = 1.f - ToFloat(uv->GetDirectArray().GetAt(vertexCounter).mData[1]);
		}
		break;
		case FbxLayerElement::EReferenceMode::eIndex:
		{

		}
		break;
		case FbxLayerElement::EReferenceMode::eIndexToDirect:
		{
			int index = uv->GetIndexArray().GetAt(vertexCounter);
			vertex.Tex0.x = ToFloat(uv->GetDirectArray().GetAt(index).mData[0]);
			vertex.Tex0.y = 1.f - ToFloat(uv->GetDirectArray().GetAt(index).mData[1]);
		}
		break;
		}
	}
	break;

	}
}

void FBXLoader::loadNormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter)
{
	FbxGeometryElementNormal *element = _pMesh->GetElementNormal(0);
	switch (element->GetMappingMode())
	{
	case FbxLayerElement::EMappingMode::eByControlPoint:
	{
		switch (element->GetReferenceMode())
		{
		case FbxLayerElement::EReferenceMode::eDirect:
		{
			vertex.Normal.x = ToFloat(element->GetDirectArray().GetAt(controlPointIndex).mData[0]);
			vertex.Normal.y = ToFloat(element->GetDirectArray().GetAt(controlPointIndex).mData[1]);
			vertex.Normal.z = ToFloat(element->GetDirectArray().GetAt(controlPointIndex).mData[2]);
		}
		break;
		case FbxLayerElement::EReferenceMode::eIndex:
		{

		}
		break;
		case FbxLayerElement::EReferenceMode::eIndexToDirect:
		{
			int index = element->GetIndexArray().GetAt(controlPointIndex);
			vertex.Normal.x = ToFloat(element->GetDirectArray().GetAt(index).mData[0]);
			vertex.Normal.y = ToFloat(element->GetDirectArray().GetAt(index).mData[1]);
			vertex.Normal.z = ToFloat(element->GetDirectArray().GetAt(index).mData[2]);
		}
		break;
		default:
		{
			DEV_ASSERT_MSG("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint");
		}
		break;
		}
	}
	break;

	case FbxLayerElement::EMappingMode::eByPolygonVertex:
	{
		switch (element->GetReferenceMode())
		{
		case FbxLayerElement::EReferenceMode::eDirect:
		{
			vertex.Normal.x = ToFloat(element->GetDirectArray().GetAt(vertexCounter).mData[0]);
			vertex.Normal.y = ToFloat(element->GetDirectArray().GetAt(vertexCounter).mData[1]);
			vertex.Normal.z = ToFloat(element->GetDirectArray().GetAt(vertexCounter).mData[2]);
		}
		break;
		case FbxLayerElement::EReferenceMode::eIndex:
		{

		}
		break;
		case FbxLayerElement::EReferenceMode::eIndexToDirect:
		{
			int index = element->GetIndexArray().GetAt(vertexCounter);
			vertex.Normal.x = ToFloat(element->GetDirectArray().GetAt(index).mData[0]);
			vertex.Normal.y = ToFloat(element->GetDirectArray().GetAt(index).mData[1]);
			vertex.Normal.z = ToFloat(element->GetDirectArray().GetAt(index).mData[2]);
		}
		break;
		}
	}
	break;

	}
}

void FBXLoader::loadTangent(Vertex &vertex, const int controlPointIndex, const int vertexCounter)
{
	FbxGeometryElementTangent *element = _pMesh->GetElementTangent(0);
	switch (element->GetMappingMode())
	{
	case FbxLayerElement::EMappingMode::eByControlPoint:
	{
		switch (element->GetReferenceMode())
		{
		case FbxLayerElement::EReferenceMode::eDirect:
		{
			vertex.Tangent.x = ToFloat(element->GetDirectArray().GetAt(controlPointIndex).mData[0]);
			vertex.Tangent.y = ToFloat(element->GetDirectArray().GetAt(controlPointIndex).mData[1]);
			vertex.Tangent.z = ToFloat(element->GetDirectArray().GetAt(controlPointIndex).mData[2]);
		}
		break;
		case FbxLayerElement::EReferenceMode::eIndex:
		{

		}
		break;
		case FbxLayerElement::EReferenceMode::eIndexToDirect:
		{
			int index = element->GetIndexArray().GetAt(controlPointIndex);
			vertex.Tangent.x = ToFloat(element->GetDirectArray().GetAt(index).mData[0]);
			vertex.Tangent.y = ToFloat(element->GetDirectArray().GetAt(index).mData[1]);
			vertex.Tangent.z = ToFloat(element->GetDirectArray().GetAt(index).mData[2]);
		}
		break;
		default:
		{
			DEV_ASSERT_MSG("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint");
		}
		break;
		}
	}
	break;

	case FbxLayerElement::EMappingMode::eByPolygonVertex:
	{
		switch (element->GetReferenceMode())
		{
		case FbxLayerElement::EReferenceMode::eDirect:
		{
			vertex.Tangent.x = ToFloat(element->GetDirectArray().GetAt(vertexCounter).mData[0]);
			vertex.Tangent.y = ToFloat(element->GetDirectArray().GetAt(vertexCounter).mData[1]);
			vertex.Tangent.z = ToFloat(element->GetDirectArray().GetAt(vertexCounter).mData[2]);
		}
		break;
		case FbxLayerElement::EReferenceMode::eIndex:
		{

		}
		break;
		case FbxLayerElement::EReferenceMode::eIndexToDirect:
		{
			int index = element->GetIndexArray().GetAt(vertexCounter);
			vertex.Tangent.x = ToFloat(element->GetDirectArray().GetAt(index).mData[0]);
			vertex.Tangent.y = ToFloat(element->GetDirectArray().GetAt(index).mData[1]);
			vertex.Tangent.z = ToFloat(element->GetDirectArray().GetAt(index).mData[2]);
		}
		break;
		}
	}
	break;

	}
}

void FBXLoader::loadBinormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter)
{
	FbxGeometryElementBinormal *element = _pMesh->GetElementBinormal(0);
	switch (element->GetMappingMode())
	{
	case FbxLayerElement::EMappingMode::eByControlPoint:
	{
		switch (element->GetReferenceMode())
		{
		case FbxLayerElement::EReferenceMode::eDirect:
		{
			vertex.Binormal.x = ToFloat(element->GetDirectArray().GetAt(controlPointIndex).mData[0]);
			vertex.Binormal.y = ToFloat(element->GetDirectArray().GetAt(controlPointIndex).mData[1]);
			vertex.Binormal.z = ToFloat(element->GetDirectArray().GetAt(controlPointIndex).mData[2]);
		}
		break;
		case FbxLayerElement::EReferenceMode::eIndex:
		{

		}
		break;
		case FbxLayerElement::EReferenceMode::eIndexToDirect:
		{
			int index = element->GetIndexArray().GetAt(controlPointIndex);
			vertex.Binormal.x = ToFloat(element->GetDirectArray().GetAt(index).mData[0]);
			vertex.Binormal.y = ToFloat(element->GetDirectArray().GetAt(index).mData[1]);
			vertex.Binormal.z = ToFloat(element->GetDirectArray().GetAt(index).mData[2]);
		}
		break;
		default:
		{
			DEV_ASSERT_MSG("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint");
		}
		break;
		}
	}
	break;

	case FbxLayerElement::EMappingMode::eByPolygonVertex:
	{
		switch (element->GetReferenceMode())
		{
		case FbxLayerElement::EReferenceMode::eDirect:
		{
			vertex.Binormal.x = ToFloat(element->GetDirectArray().GetAt(vertexCounter).mData[0]);
			vertex.Binormal.y = ToFloat(element->GetDirectArray().GetAt(vertexCounter).mData[1]);
			vertex.Binormal.z = ToFloat(element->GetDirectArray().GetAt(vertexCounter).mData[2]);
		}
		break;
		case FbxLayerElement::EReferenceMode::eIndex:
		{

		}
		break;
		case FbxLayerElement::EReferenceMode::eIndexToDirect:
		{
			int index = element->GetIndexArray().GetAt(vertexCounter);
			vertex.Binormal.x = ToFloat(element->GetDirectArray().GetAt(index).mData[0]);
			vertex.Binormal.y = ToFloat(element->GetDirectArray().GetAt(index).mData[1]);
			vertex.Binormal.z = ToFloat(element->GetDirectArray().GetAt(index).mData[2]);
		}
		break;
		}
	}
	break;

	}
}

void FBXLoader::loadAnimation()
{

}

void FBXLoader::loadSkeletonNode(fbxsdk::FbxNode *pNode, const char* parentName)
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

void FBXLoader::loadTexture()
{
	for (uint32 i = 0; i < _materialCount; ++i)
	{
		FbxSurfaceMaterial *pSurfaceMaterial = _pScene->GetMaterial(static_cast<int>(i));
		if (nullptr == pSurfaceMaterial)
			continue;

		_texturesList.push_back(TextureList(enumToIndex(TextureType::End), nullptr));
		for (uint32 j = 0; j < enumToIndex(TextureType::End); ++j)
		{
			const char *propertyString = getSurfacePropertyString(CastValue<TextureType>(j));
			if (nullptr != propertyString)
			{
				loadTexture(&pSurfaceMaterial->FindProperty(propertyString), CastValue<TextureType>(j));
			}
		}
	}
}

void FBXLoader::loadTexture(FbxProperty *property, const TextureType textureType)
{
	int layeredTextureCount = property->GetSrcObjectCount<FbxLayeredTexture>();
	if (0 < layeredTextureCount)
	{
		for (int j = 0; j < layeredTextureCount; ++j)
		{
			FbxLayeredTexture *layeredTexture = FbxCast<FbxLayeredTexture>(property->GetSrcObject<FbxLayeredTexture>(j));
			int textureCount = layeredTexture->GetSrcObjectCount<FbxTexture>();
			for (int k = 0; k < textureCount; ++k)
			{
				FbxTexture *texture = layeredTexture->GetSrcObject<FbxTexture>(k);
				FbxFileTexture *fileTexture = FbxCast<FbxFileTexture>(texture);

				const char *a = fileTexture->GetFileName();
				int b = 0;
			}
		}
	}
	else
	{
		int textureCount = property->GetSrcObjectCount<FbxTexture>();
		for (int j = 0; j < textureCount; ++j)
		{
			FbxTexture *texture = property->GetSrcObject<FbxTexture>(j);
			FbxFileTexture *fileTexture = FbxCast<FbxFileTexture>(texture);

			char filePathName[MAX_PATH] = "";
			strcpy_s(filePathName, _filePath.c_str());
			strcat_s(filePathName, sizeof(char) * MAX_PATH, PathFindFileNameA(fileTexture->GetFileName()));

			std::shared_ptr<TextureComponent> pTextureComponent = std::make_shared<TextureComponent>(filePathName);
			
			TextureList &textureList = _texturesList.back();
			textureList[enumToIndex(textureType)] = pTextureComponent;
		}
	}
}

const char* FBXLoader::getSurfacePropertyString(const TextureType textureType)
{	
	FbxProperty a;
	switch (textureType)
	{
	case TextureType::Diffuse:
		return FbxSurfaceMaterial::sDiffuse;
	case TextureType::Normal:
		return FbxSurfaceMaterial::sBump;
	case TextureType::Specular:
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
