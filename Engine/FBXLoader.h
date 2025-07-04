#pragma once

#include "Include.h"

#include "FBXSDK/fbxsdk.h"
#include "Vertex.h"
#include "Mesh/Mesh.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <future>

class ThreadPool {
public:
	ThreadPool(size_t num_threads);
	~ThreadPool();

	// job 을 추가한다.
	template <class F, class... Args>
	std::future<std::invoke_result_t<F, Args...>> EnqueueJob(
		F&& f, Args&&... args);

private:
	// 총 Worker 쓰레드의 개수.
	size_t num_threads_;
	// Worker 쓰레드를 보관하는 벡터.
	std::vector<std::thread> worker_threads_;
	// 할일들을 보관하는 job 큐.
	std::queue<std::function<void()>> jobs_;
	// 위의 job 큐를 위한 cv 와 m.
	std::condition_variable cv_job_q_;
	std::mutex m_job_q_;

	// 모든 쓰레드 종료
	bool stop_all;

	// Worker 쓰레드
	void WorkerThread();
};

template <class F, class... Args>
std::future<std::invoke_result_t<F, Args...>> ThreadPool::EnqueueJob(
	F&& f, Args&&... args) {
	if (stop_all) {
		throw std::runtime_error("ThreadPool 사용 중지됨");
	}

	using return_type = std::invoke_result_t<F, Args...>;
	auto job = std::make_shared<std::packaged_task<return_type()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...));
	std::future<return_type> job_result_future = job->get_future();
	{
		std::lock_guard<std::mutex> lock(m_job_q_);
		jobs_.push([job]() { (*job)(); });
	}
	cv_job_q_.notify_one();

	return job_result_future;
}

class PerformanceTimer
{
public:
	PerformanceTimer(const std::wstring &name = TEXT(""))
		: _start{ std::chrono::system_clock::now() }
		, _name(name)
	{

	}

	~PerformanceTimer()
	{
		std::chrono::duration<double> sec = std::chrono::system_clock::now() - _start;
		std::wstring timeLog = _name + std::to_wstring(sec.count()) + TEXT("\r\n");
		OutputDebugStringW(timeLog.c_str());
	}

	std::wstring _name;
	std::chrono::system_clock::time_point _start;
};

struct FVertexKey
{
    FVertexKey() = default;
    FVertexKey(const FVertexKey& Rhs) = default;
    FVertexKey(int a0, int a, int b, int c, int d)
        : ControlPointIndex(a0), UVIndex(a), NormalIndex(b), TangentIndex(c), BiNormalIndex(d)
    {

    }
    FVertexKey(FVertexKey&& Rhs) = default;

    int ControlPointIndex = -1;
    int UVIndex = -1;
    int NormalIndex = -1;
    int TangentIndex = -1;
    int BiNormalIndex = -1;

    bool operator==(const FVertexKey& Rhs) const
    {
        return ControlPointIndex == Rhs.ControlPointIndex && UVIndex == Rhs.UVIndex && NormalIndex == Rhs.NormalIndex && TangentIndex == Rhs.TangentIndex && BiNormalIndex == Rhs.BiNormalIndex;
    }
};

namespace std
{
    template<>
    struct hash<FVertexKey>
    {
        size_t operator()(const FVertexKey& Data) const
        {
            size_t h0 = std::hash<int>{}(Data.ControlPointIndex);
            size_t h1 = std::hash<int>{}(Data.UVIndex);
            size_t h2 = std::hash<int>{}(Data.NormalIndex);
            size_t h3 = std::hash<int>{}(Data.TangentIndex);
            size_t h4 = std::hash<int>{}(Data.BiNormalIndex);

            size_t h = h0;
            h ^= h1 + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= h2 + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= h3 + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= h4 + 0x9e3779b9 + (h << 6) + (h >> 2);

            return h;
        }
    };
}

class MTexture;

class ENGINE_DLL MFBXLoader
{
public:
	explicit MFBXLoader();
	explicit MFBXLoader(const wchar_t *filePathName);
	~MFBXLoader();
protected:
    void SafeDestroy(fbxsdk::FbxObject*& InObject);


public:
	void LoadAnim(std::vector<AnimationClip>& animationClipList);
	bool LoadMesh(const std::wstring& InPath);
    // FBX파일을 Json으로 만들 때 호출됨
    void SaveJsonAsset(const std::wstring& InPath);
private:
	void InitializeFbxSdk();
	void convertScene();

public:
    const std::wstring& GetDirectory() const { return Directory; }
private:
	std::wstring Path;
	std::wstring Directory;
    std::wstring Name;
    std::wstring Extension;

private:
	void initializeSDK();
public:
	const uint32 getJointCount() const;
private:
	static fbxsdk::FbxManager		*_pFbxManager;
	fbxsdk::FbxImporter				*_pImporter = nullptr;
	fbxsdk::FbxScene				*_pScene = nullptr;

public:
	// 애니메이션 관련 데이터
	fbxsdk::FbxSkeleton				*_pSkeleton = nullptr;
	fbxsdk::FbxAnimStack			*_pAnimStack = nullptr;
	std::vector<FJoint>				Joints;
    // 조인트의 [이름, 인덱스] 쌍을 저장함
	JointIndexMap					JointIndices;
	VertexWeightInfoListMap			_vertexWeightInfoListMap;

private:
	void loadNode();
private:
	void parseMeshNode(fbxsdk::FbxNode *pNode, const uint32 meshIndex);
	void loadPosition(Vertex &vertex, const int controlPointIndex);
	void loadUV(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FVertexKey& VertexKey);
	void loadNormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FVertexKey& VertexKey);
	void loadTangent(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FVertexKey& VertexKey);
	void loadBinormal(Vertex &vertex, const int controlPointIndex, const int vertexCounter, FVertexKey& VertexKey);
	void loadAnimation();
private:
	void loadSkeletonNode(fbxsdk::FbxNode *pNode, const char* parentName);
private:
	void loadTexture();
private:
    // 메시와 
	void linkMaterial(fbxsdk::FbxNode *pNode);
private:
	fbxsdk::FbxMesh *FBXMesh;
	std::vector<fbxsdk::FbxMesh*> _meshList;
	std::set<fbxsdk::FbxCluster*> clustermap;
private:
	void LoadTexturesFromFBXMaterial(FbxSurfaceMaterial* SurfaceMaterial, uint32 MaterialIndex);
	const char* GetTexturePropertyString(ETextureType TextureType);

public:
    // 각 메시의 버텍스 리스트
	std::vector<VertexList>&	getVerticesList();
    // 각 메시의 인덱스 리스트
	std::vector<IndexList>&		getIndicesList();
    // 각 메시의 텍스쳐 리스트
	std::vector<TextureList>&   GetTextures();
	const std::vector<uint32>&	GetMaterialIndices() const;
private:
	std::vector<VertexList>		_verticesList;
	std::vector<IndexList>		_indicesList;
    // ControlPoint에 해당하는 정점들의 인덱스를 메시 단위로 저장
	std::vector<std::map<int, std::vector<int>>> ControlPointToVertexIndices;
	
    // 매터리얼별 텍스쳐 리스트
	std::vector<TextureList>	MaterialTextures;

	//// 사용된 텍스쳐
	//std::vector<MTexture> _texturesList;

	//// 매터리얼 - 텍스처 인덱스 바인딩
	//std::vector<uint32, std::vector<uint32>> MaterialTextureIndicesMap;

    // 각 메시의 매터리얼 인덱스
	std::vector<uint32>			MaterialIndices;
	int meshCounter;

public:
	void getBoundingBoxInfo(Vec3 &min, Vec3 &max) { min = MinPosition, max = MaxPosition; }
private:
	// 바운딩 박스용
	Vec3 MinPosition;
	Vec3 MaxPosition;

public:
	const uint32 GetGeometryNum() const;
	const uint32 GetMaterialNum() const;
private:
	uint32 GeometryCount;
	uint32 MaterialNum;

public:
    std::wstring GetMaterialIName(uint32 Index);

public:
	const uint32 GetTotalVertexNum() const;
private:
	uint32 TotalVertexNum;
};

inline DirectX::XMMATRIX ToXMMatrix(const FbxAMatrix& pSrc);
