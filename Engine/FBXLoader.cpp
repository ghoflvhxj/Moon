#include "stdafx.h"
#include "FBXLoader.h"

#include "TextureComponent.h"

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
{
	initializeFbxSdk();
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
{
	initializeFbxSdk();
	loadModel();

	//---------------------------------------------------------------
	// process joint and animation
	int animStackCount = _pImporter->GetAnimStackCount();
	animationClipList.resize(animStackCount);

	for (int animStackIndex = 0; animStackIndex < animStackCount; ++animStackIndex)
	{
		_pAnimStack = _pScene->GetCurrentAnimationStack();
		FbxString animStackName = _pAnimStack->GetName();
		FbxTakeInfo* pTakeInfo = _pScene->GetTakeInfo(animStackName);
		FbxTime startTime = pTakeInfo->mLocalTimeSpan.GetStart();
		FbxTime endTime = pTakeInfo->mLocalTimeSpan.GetStop();

		animationClipList[animStackIndex]._keyFrameLists.resize(_jointList.size());
		animationClipList[animStackIndex]._animationName = animStackName.Buffer();
		animationClipList[animStackIndex]._startFrame = ToUint32(startTime.GetFrameCount(FbxTime::eFrames24));
		animationClipList[animStackIndex]._endFrame = ToUint32(endTime.GetFrameCount(FbxTime::eFrames24));
		animationClipList[animStackIndex]._frameCount = ToUint32((endTime - startTime).GetFrameCount(FbxTime::eFrames24));
		animationClipList[animStackIndex]._duration = (endTime - startTime).GetSecondDouble();

		uint32 meshCount = sizeToUint32(_meshList.size());
		for (int meshIndex = 0; meshIndex < meshCount; ++meshIndex)
		{
			FbxSkin *pSkin = nullptr;
			int deformerCount = _meshList[meshIndex]->GetDeformerCount();
			for (int index = 0; index < deformerCount; ++index)
			{
				FbxDeformer *pDeformer = _pMesh->GetDeformer(index);
				if (nullptr == pDeformer)
				{
					continue;
				}

				if (FbxDeformer::EDeformerType::eSkin != pDeformer->GetDeformerType())
				{
					continue;
				}

				FbxSkin *pTempSkin = reinterpret_cast<FbxSkin*>(_pMesh->GetDeformer(index));
				if (nullptr == pTempSkin)
				{
					continue;
				}

				pSkin = pTempSkin;
			}

			if (nullptr == pSkin)
			{
				continue;
			}

			FbxNode *pMeshNode = _meshList[meshIndex]->GetNode(0);
			FbxAMatrix geometryTransform = { pMeshNode->GetGeometricTranslation(FbxNode::EPivotSet::eSourcePivot),
												pMeshNode->GetGeometricRotation(FbxNode::EPivotSet::eSourcePivot),
												pMeshNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot) };

			int clusterCount = pSkin->GetClusterCount();
			for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
			{
				FbxCluster *pCluster = pSkin->GetCluster(clusterIndex);
				FbxNode *pNode = pCluster->GetLink();

				// 조인트의 바인드포즈 인버스 매트릭스 얻기
				FbxAMatrix transformMatrix;
				FbxAMatrix transformLinkMatrix;
				FbxAMatrix globalBindPoseInverseMatrix;

				pCluster->GetTransformMatrix(transformMatrix);
				pCluster->GetTransformLinkMatrix(transformLinkMatrix);
				globalBindPoseInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;
				// VertexAtTimeT = TransformationOfPoseAtTimeT * InverseOfGlobalBindPoseMatrix * VertexAtBindingTime 
				// 에서 InverseOfGlobalBindPoseMatrix를 얻어냈음

				const char* jointName = pNode->GetName();
				if (_jointIndexMap.end() == _jointIndexMap.find(jointName))
				{
					DEV_ASSERT_MSG("찾을 수 없는 Bone입니다!")
						continue;
				}

				int jointIndex = _jointIndexMap[jointName];
				XMStoreFloat4x4(&_jointList[jointIndex]._inverseOfGlobalBindPoseMatrix, ToXMMatrix(globalBindPoseInverseMatrix));

				// 본(조인트)에 영향을 받는 컨트롤 포인트를 얻음
				int controlPointIndexCount = pCluster->GetControlPointIndicesCount();
				int *controlPointIndexArray = pCluster->GetControlPointIndices();
				for (int j = 0; j < controlPointIndexCount; ++j)
				{
					if (_vertexWeightInfoListMap.end() == _vertexWeightInfoListMap.find(controlPointIndexArray[j]))
					{
						_vertexWeightInfoListMap.emplace(controlPointIndexArray[j], VertexIndexWeightInfo());
					}

					_vertexWeightInfoListMap[controlPointIndexArray[j]]._jointIndexList.push_back(jointIndex);
					_vertexWeightInfoListMap[controlPointIndexArray[j]]._weightList.push_back(pCluster->GetControlPointWeights());
				}

				// 프레임에 존재하는 키 프레임을 저장해야 함
				animationClipList[animStackIndex]._keyFrameLists[jointIndex].reserve(animationClipList[animStackIndex]._frameCount);
				for (FbxLongLong i = startTime.GetFrameCount(FbxTime::eFrames24); i <= endTime.GetFrameCount(FbxTime::eFrames24); ++i)
				{
					FbxTime currentTime;
					currentTime.SetFrame(i, FbxTime::eFrames24);

					FbxAMatrix currentTransformOffset = pMeshNode->EvaluateGlobalTransform(currentTime) * geometryTransform;	// 메시의 글로벌 트랜스폼 * 지오메트리 트랜스폼

					KeyFrame keyFrame = {};
					XMStoreFloat4x4(&keyFrame._matrix, ToXMMatrix(currentTransformOffset.Inverse() * pCluster->GetLink()->EvaluateGlobalTransform(currentTime)));

#ifdef _DEBUG
					keyFrame._scale = Vec3{ ToFloat(pNode->LclScaling.Get().mData[0])
												  , ToFloat(pNode->LclScaling.Get().mData[1])
												  , ToFloat(pNode->LclScaling.Get().mData[2]) };
					keyFrame._rotation = Vec3{ ToFloat(pNode->LclRotation.Get().mData[0])
												  , ToFloat(pNode->LclRotation.Get().mData[1])
												  , ToFloat(pNode->LclRotation.Get().mData[2]) };
					keyFrame._translation = Vec3{ ToFloat(pNode->LclTranslation.Get().mData[0])
												  , ToFloat(pNode->LclTranslation.Get().mData[1])
												  , ToFloat(pNode->LclTranslation.Get().mData[2]) };
#endif
					animationClipList[animStackIndex]._keyFrameLists[jointIndex].push_back(keyFrame);
				}
			}
		}

	}

	int test = 0;
}

FBXLoader::~FBXLoader()
{
	_pImporter->Destroy();
	_pScene->Destroy();
}

void FBXLoader::initializeFbxSdk()
{
	_filePath = _filePath.substr(0, _filePath.find_last_of('/') + 1);

	if (nullptr == _pFbxManager)
		initializeSDK();

	_pImporter = FbxImporter::Create(_pFbxManager, "");
	if (nullptr == _pImporter)
	{
		DEV_ASSERT_MSG(TEXT("FbxImporter가 nullptr 입니다!"));
	}

	if (false == _pImporter->Initialize(_filePathName.c_str()))
	{
		DEV_ASSERT_MSG(TEXT("FbxImporter 초기화에 실패했습니다!"));
	}

	_pScene = FbxScene::Create(_pFbxManager, "Scene");
	if (nullptr == _pScene)
	{
		DEV_ASSERT_MSG(TEXT("FbxScene이 nullptr  입니다!"));
	}

	if (false == _pImporter->Import(_pScene))
	{
		DEV_ASSERT_MSG(TEXT("FbxImporter가 FbxScene을 불러오지 못했습니다!"));
	}
}

void FBXLoader::loadModel()
{
	_geometryCount = _pScene->GetGeometryCount();
	_verticesList.reserve(_geometryCount);
	_indicesList.reserve(_geometryCount);
	_linkList.reserve(_geometryCount);

	_materialCount = static_cast<uint32>(_pScene->GetMaterialCount());
	_texturesList.reserve(_materialCount);

	FbxGeometryConverter geometryConverter(_pFbxManager);
	geometryConverter.Triangulate(_pScene, true);

	loadNode();
	loadTexture();
}

void FBXLoader::initializeSDK()
{
	_pFbxManager = FbxManager::Create();

	FbxIOSettings *ios = FbxIOSettings::Create(_pFbxManager, IOSROOT);
	_pFbxManager->SetIOSettings(ios);
}

const uint32 FBXLoader::getGeometryCount() const
{
	return _geometryCount;
}

const uint32 FBXLoader::getMaterialCount() const
{
	return _materialCount;
}

std::vector<VertexList> &FBXLoader::getVertices()
{
	return _verticesList;
}

std::vector<IndexList> &FBXLoader::getIndices()
{
	return _indicesList;
}

std::vector<TextureList> &FBXLoader::getTextures()
{
	return _texturesList;
}

const std::vector<int>& FBXLoader::getLinkList() const
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
				loadMeshNode(pNode);
				break;
			case FbxNodeAttribute::EType::eSkeleton:
				loadSkeletonNode(pNode);
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

void FBXLoader::loadMeshNode(FbxNode *pNode)
{
	_pMesh = pNode->GetMesh();
	_meshList.push_back(_pMesh);

	_verticesList.push_back(VertexList());
	_indicesList.push_back(IndexList());

	int polygonCount = _pMesh->GetPolygonCount();
	int vertexCounter = 0;							// 맵핑 모드가 eByPolygonVertex일 경우 사용함
	int vertexCount = polygonCount * 3;
	_verticesList.back().reserve(polygonCount * 3);
	_indicesList.back().reserve(polygonCount * 3);

	linkMaterial(pNode);

	for (int i = 0; i < polygonCount; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			int controlPointIndex = _pMesh->GetPolygonVertex(i, j);

			Vertex vertex;
			loadPosition(vertex, controlPointIndex);
			loadUV(vertex, controlPointIndex, vertexCounter);
			loadNormal(vertex, controlPointIndex, vertexCounter);
			loadTangent(vertex, controlPointIndex, vertexCounter);
			loadBinormal(vertex, controlPointIndex, vertexCounter);
			_verticesList.back().push_back(vertex);
			_indicesList.back().push_back((3 * i) + j);

			++vertexCounter;
		}
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
	vertex.Pos.x = ToFloat(_pMesh->GetControlPointAt(controlPointIndex).mData[0]);
	vertex.Pos.y = ToFloat(_pMesh->GetControlPointAt(controlPointIndex).mData[1]);
	vertex.Pos.z = ToFloat(_pMesh->GetControlPointAt(controlPointIndex).mData[2]);
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
			vertex.Tex0.y = ToFloat(uv->GetDirectArray().GetAt(controlPointIndex).mData[1]);
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
			DEV_ASSERT_MSG(TEXT("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint"));
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
			DEV_ASSERT_MSG(TEXT("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint"));
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
			DEV_ASSERT_MSG(TEXT("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint"));
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
			DEV_ASSERT_MSG(TEXT("Fbx 파일에서 찾을 수 없는 UV입니다. EMappingMode::eByControlPoint"));
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

void FBXLoader::loadSkeletonNode(fbxsdk::FbxNode *pNode)
{
	_jointIndexMap.emplace(pNode->GetName(), static_cast<uint32>(_jointList.size()));

	Joint joint;
	joint._name			= pNode->GetName();

	_jointList.push_back(joint);
}

void FBXLoader::loadTexture()
{
	for (uint32 i = 0; i < _materialCount; ++i)
	{
		FbxSurfaceMaterial *pSurfaceMaterial = _pScene->GetMaterial(static_cast<int>(i));
		if (nullptr == pSurfaceMaterial)
			continue;

		_texturesList.push_back(TextureList(enumToUInt32(TextureType::End), nullptr));
		for (uint32 j = 0; j < enumToUInt32(TextureType::End); ++j)
		{
			const char *propertyString = getSurfacePropertyString(uint32ToEnum<TextureType>(j));
			if (nullptr != propertyString)
			{
				loadTexture(&pSurfaceMaterial->FindProperty(propertyString), uint32ToEnum<TextureType>(j));
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

DirectX::XMMATRIX ToXMMatrix(const FbxAMatrix& pSrc)
{
	return DirectX::XMMatrixSet(
		static_cast<FLOAT>(pSrc.Get(0, 0)), static_cast<FLOAT>(pSrc.Get(0, 1)), static_cast<FLOAT>(pSrc.Get(0, 2)), static_cast<FLOAT>(pSrc.Get(0, 3)),
		static_cast<FLOAT>(pSrc.Get(1, 0)), static_cast<FLOAT>(pSrc.Get(1, 1)), static_cast<FLOAT>(pSrc.Get(1, 2)), static_cast<FLOAT>(pSrc.Get(1, 3)),
		static_cast<FLOAT>(pSrc.Get(2, 0)), static_cast<FLOAT>(pSrc.Get(2, 1)), static_cast<FLOAT>(pSrc.Get(2, 2)), static_cast<FLOAT>(pSrc.Get(2, 3)),
		static_cast<FLOAT>(pSrc.Get(3, 0)), static_cast<FLOAT>(pSrc.Get(3, 1)), static_cast<FLOAT>(pSrc.Get(3, 2)), static_cast<FLOAT>(pSrc.Get(3, 3)));
}
