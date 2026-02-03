#pragma once

#include "Module/Graphic/Shader/Shader.h"

class MVertexShader : public MShader
{
public:
    explicit MVertexShader(const std::wstring& filePathName);
    explicit MVertexShader();
    virtual ~MVertexShader();

public:
    virtual void SetToDevice() override;

public:
    ID3D11VertexShader* getRaw();
private:
    ID3D11VertexShader* _pVertexShader;
};