#include "Include.h"
#include "TextureSetter.h"

#include "GraphicDevice.h"

#include "TextureComponent.h"

TextureSetter::TextureSetter(std::vector<std::shared_ptr<MTexture>> &textureList)
	: _textureCount{ 0u }
{
	_textureCount = static_cast<uint32>(textureList.size());

	for (uint32 i = 0; i < _textureCount; ++i)
	{
		if (nullptr == textureList[i])
			continue;

		textureList[i]->setTexture(i);                                        
	}
}

TextureSetter::~TextureSetter()
{
	for (uint32 i = 0; i < _textureCount; ++i)
	{
		ID3D11ShaderResourceView *pNullShaderResouceView = nullptr;
		g_pGraphicDevice->getContext()->PSSetShaderResources(i, 1, &pNullShaderResouceView);
	}
}