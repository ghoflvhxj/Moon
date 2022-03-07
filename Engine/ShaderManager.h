#pragma once
#ifndef __SHADER_MANAGER_H__

class ShaderManager
{
public:
	enum class ShaderType : uint8
	{
		Vertex, Pixel, End
	};

	using ShaderMap				= std::unordered_map<std::wstring, ID3D11DeviceChild*>;
	using ShaderReflectionMap	= std::unordered_map<std::wstring, ID3D11ShaderReflection*>;
	using BlobMap				= std::unordered_map<std::wstring, ID3D10Blob*>;

public:
	explicit ShaderManager();
	~ShaderManager();

public:
	void							releaseShader();
private:	// implementation
	const bool						addShader(const ShaderType type, const wchar_t *fileName, ID3D11DeviceChild *pShader, ID3D10Blob *pBlob);
	ShaderMap&						getShaderMap(const ShaderType type);
	const bool 						getShader(const ShaderType type, const wchar_t *fileName, ID3D11DeviceChild **pShader);
public:		// interface
	const bool						getVertexShader(const wchar_t *fileName, ID3D11VertexShader **pShader);
	const bool						addVertexShader(const wchar_t *fileName, ID3D11VertexShader *pShader, ID3D10Blob *pBlob);
public:		// interface
	const bool						getPixelShader(const wchar_t *fileName, ID3D11PixelShader **pShader);
	const bool						addPixelShader(const wchar_t *fileName, ID3D11PixelShader *pShader, ID3D10Blob *pBlob);
private:
	std::vector<ShaderMap> _shaderMapList;

public:
	ID3D10Blob*						getVertexShaderBlob(const wchar_t *shaderName);
	void							releaseBlob();	// InputLayout ������ ���� ���ܵξ��� Blob�� �Ҹ��ŵ�ϴ�.
public:
	inline BlobMap&					getBlobMap(const ShaderType type);
private:
	std::vector<BlobMap> _blobMapList;

public:
	ShaderReflectionMap&			getReflectionMap(const ShaderType type);
private:
	std::vector<ShaderReflectionMap> _reflectionMapList;
};

#define __SHADER_MANAGER_H__
#endif