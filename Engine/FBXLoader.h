#pragma once
#ifndef __FBXLOADER_H__

#include <fbxsdk.h>
#include <FBXSDK/scene/fbxaxissystem.h>
#include "Vertex.h"

#include "DynamicMeshComponentUtility.h"

#include <set>

class TextureComponent;

class Skeleton
{

};

class Mesh
{

};

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
public:
	const uint32 getJointCount() const;
private:
	static fbxsdk::FbxManager		*_pFbxManager;
	fbxsdk::FbxImporter				*_pImporter;
	fbxsdk::FbxScene				*_pScene;

public:
	// 애니메이션 관련 데이터
	fbxsdk::FbxSkeleton				*_pSkeleton;
	fbxsdk::FbxAnimStack			*_pAnimStack;
	std::vector<FJoint>				_jointList;
	JointIndexMap					_jointIndexMap;
	VertexWeightInfoListMap			_vertexWeightInfoListMap;

private:
	void loadNode();
private:
	void parseMeshNode(fbxsdk::FbxNode *pNode, const uint32 meshIndex);
	void loadPosition(Vertex &vertex, const int controlPointIndex);
	void loadUV(Vertex &vertex, const int controlPointIndex, const int vertexCounter);
	void loadNormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter);
	void loadTangent(Vertex &vertex, const int controlPointIndex, const int vertexCounter);
	void loadBinormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter);
	void loadAnimation();
private:
	void loadSkeletonNode(fbxsdk::FbxNode *pNode, const char* parentName);
private:
	void loadTexture();
private:
	void linkMaterial(fbxsdk::FbxNode *pNode);
private:
	fbxsdk::FbxMesh *_pMesh;
	std::vector<fbxsdk::FbxMesh*> _meshList;
	std::set<fbxsdk::FbxCluster*> clustermap;
private:
	void loadTexture(fbxsdk::FbxProperty *property, const TextureType textureType);
	const char* getSurfacePropertyString(const TextureType textureType);

public:
	std::vector<VertexList>&	getVerticesList();
	std::vector<IndexList>&		getIndicesList();
	std::vector<TextureList>&	getTextures();
	const std::vector<uint32>&	getLinkList() const;
private:
	std::vector<VertexList>		_verticesList;
	std::vector<IndexList>		_indicesList;
	std::vector<std::map<int, std::vector<int>>> _indexMap;
	std::vector<TextureList>	_texturesList;
	std::vector<uint32>			_linkList;
	int meshCounter;

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