#pragma once
#ifndef __MAINGAME_SETTING_H__

class MainGameSetting
{
public:
	explicit MainGameSetting();
	~MainGameSetting();
	
public:
	const float getAspectRatio() const;
	const int getResolutionWidth() const;
	const int getResolutionHeight() const;
private:
	int _resolutionWidth;
	int _resolutionHeight;

public:
	const float getFov() const;
private:
	float _fov;
};

#define __MAINGAME_SETTING_H__
#endif