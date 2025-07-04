#pragma once
#ifndef __TEXTURE_SETTER_H__

class MTexture;

class TextureSetter
{
public:
	explicit TextureSetter(std::vector<std::shared_ptr<MTexture>> &textureList);
	explicit TextureSetter() = delete;
	~TextureSetter();

private:
	uint32 _textureCount;
};

#define __TEXTURE_SETTER_H__
#endif