#include "stdafx.h"
#include "MainGameSetting.h"

MainGameSetting::MainGameSetting()
	: _resolutionWidth{ 1920 }
	, _resolutionHeight{ 1080 }
	, _fov{ 70.f }
	, _bLighting{ TRUE }
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

const BOOL MainGameSetting::getLighting() const
{
	return _bLighting;
}

const float MainGameSetting::getFov() const
{
	return _fov;
}
