#pragma once
#ifndef __INPUT_LAYOUT_H__

class InputLayout
{
public:
	explicit InputLayout(D3D11_INPUT_ELEMENT_DESC desc[], const uint32 elementNum, const wchar_t *vertexShader, const wchar_t *pixelShader);
	~InputLayout();

private:
	ID3D11InputLayout	*_pInputLayout;
};

#define __INPUT_LAYOUT_H__
#endif