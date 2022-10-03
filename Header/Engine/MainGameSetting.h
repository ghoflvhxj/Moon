#pragma once
#ifndef __MAINGAME_SETTING_H__

class ENGINE_DLL MainGameSetting
{
public:
	explicit MainGameSetting();
	~MainGameSetting();
	
public:
	const float getAspectRatio() const;
	const int getResolutionWidth() const;
	const int getResolutionHeight() const;
	const BOOL getLighting() const;
private:
	int _resolutionWidth;
	int _resolutionHeight;
	BOOL _bLighting;
public:
	const float getFov() const;
private:
	float _fov;
};

#define __MAINGAME_SETTING_H__
#endif