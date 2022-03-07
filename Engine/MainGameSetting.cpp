#include "stdafx.h"
#include "MainGameSetting.h"

MainGameSetting::MainGameSetting()
	: _resolutionWidth{ 1600 }
	, _resolutionHeight{ 900 }
	, _fov{ 75.f }
{
}

MainGameSetting::~MainGameSetting()
{
}

const float MainGameSetting::getAspectRatio() const
{
	return static_cast<float>(_resolutionWidth) / _resolutionHeight;
}

const int MainGameSetting::getResolutionWidth() const
{
	return _resolutionWidth;
}

const int MainGameSetting::getResolutionHeight() const
{
	return _resolutionHeight;
}

const float MainGameSetting::getFov() const
{
	return _fov;
}
