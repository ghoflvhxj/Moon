#pragma once
#ifndef __SHADER_MANAGER_H__

class ShaderManager
{
public:
	enum class ShaderType : uint8
	{
		Vertex, Pixel, End
	};

	using ShaderMap = std::unordered_map<std::wstring, ID3D11DeviceChild*>;
	using BlobMap	= std::unordered_map<std::wstring, ID3D10Blob*>;

public:
	explicit ShaderManager();
	~ShaderManager();

public:
	void							releaseShader();
private:						
	inline ShaderMap&				getShaderMap(const ShaderType type);
	inline const bool 				getShader(const ShaderType type, const wchar_t *fileName, ID3D11DeviceChild **pShader);
public:
	const bool						getVertexShader(const wchar_t *fileName, ID3D11VertexShader **pShader);
	const bool						addVertexShader(const wchar_t *fileName, ID3D11VertexShader *pShader, ID3D10Blob *pBlob);
public:
	const bool						getPixelShader(const wchar_t *fileName, ID3D11PixelShader **pShader);
	const bool						addPixelShader(const wchar_t *fileName, ID3D11PixelShader *pShader, ID3D10Blob *pBlob);
private:
	std::vector<ShaderMap> _shaderMapList;

public:
	ID3D10Blob*						getVertexShaderBlob(const wchar_t *shaderName);
	void							releaseBlob();	// InputLayout 생성을 위해 남겨두었던 Blob을 소멸시킵니다.
private:
	inline BlobMap&					getBlobMap(const ShaderType type);
private:
	std::vector<BlobMap> _blobMapList;
};

#define __SHADER_MANAGER_H__
#endif