#pragma once
#ifndef __FBXLOADER_H__

#include <fbxsdk.h>
#include <FBXSDK/scene/fbxaxissystem.h>
#include "Vertex.h"

#include "DynamicMeshComponentUtility.h"

class TextureComponent;

class FBXLoader
{
public:
	explicit FBXLoader(const char *filePathName);
	explicit FBXLoader(const char *filePathName, std::vector<AnimationClip> &animationClipList);
	~FBXLoader();
private:
	void initializeFbxSdk();
	void convertScene();
	void loadModel();

private:
	std::string _filePathName;
	std::string _filePath;

private:
	void initializeSDK();
private:
	static fbxsdk::FbxManager		*_pFbxManager;
	fbxsdk::FbxImporter				*_pImporter;
	fbxsdk::FbxScene				*_pScene;

	// 애니메이션 관련
	fbxsdk::FbxSkeleton				*_pSkeleton;
	fbxsdk::FbxAnimStack			*_pAnimStack;

	// 애니메이션 컨트롤러
	std::vector<Joint>				_jointList;
	JointIndexMap					_jointIndexMap;
	VertexWeightInfoListMap			_vertexWeightInfoListMap;

private:
	void loadNode();
private:
	void parseMeshNode(fbxsdk::FbxNode *pNode);
	void loadPosition(Vertex &vertex, const int controlPointIndex);
	void loadUV(Vertex &vertex, const int controlPointIndex, const int vertexCounter);
	void loadNormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter);
	void loadTangent(Vertex &vertex, const int controlPointIndex, const int vertexCounter);
	void loadBinormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter);
	void loadAnimation();
private:
	void loadSkeletonNode(fbxsdk::FbxNode *pNode);
private:
	void loadTexture();
private:
	void linkMaterial(fbxsdk::FbxNode *pNode);
private:
	fbxsdk::FbxMesh *_pMesh;
	std::vector<fbxsdk::FbxMesh*> _meshList;

private:
	void loadTexture(fbxsdk::FbxProperty *property, const TextureType textureType);
	const char* getSurfacePropertyString(const TextureType textureType);

public:
	std::vector<VertexList>&	getVerticesList();
	std::vector<IndexList>&		getIndicesList();
	std::vector<TextureList>&	getTextures();
	const std::vector<int>&		getLinkList() const;
private:
	std::vector<VertexList>		_verticesList;
	std::vector<IndexList>		_indicesList;
	std::vector<TextureList>	_texturesList;
	std::vector<int>			_linkList;

public:
	const uint32 getGeometryCount() const;
	const uint32 getMaterialCount() const;
private:
	uint32 _geometryCount;
	uint32 _materialCount;

public:
	const uint32 getVertexCount() const;
private:
	uint32 _vertexCount;
};

DirectX::XMMATRIX ToXMMatrix(const FbxAMatrix& pSrc);

#define __FBXLOADER_H__
#endif