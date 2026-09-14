#pragma once

#include "IniParser.h"

#define INI_PATH "ini/editor.ini"

class FIniConfig
{
public:
	FIniConfig();

	float	GetGridOffset();
	int32	GetGridRange();
	float	GetCameraSensitivity();
	float	GetCameraSpeed();
	
	void	SetGridOffset(float _Offset);	
	void	SetGridRange(int32 _Range);	
	void	SetCameraSensitivity(float _Sensitivity);
	void	SetCameraSpeed(float _Speed);

	void	Save();
	

private:
	FIniParser IniParser;
	//Grid
	float	GridOffset;
	int32	GridRange;
	//Camera
	float	CameraSensitivity;
	float	CameraSpeed;
	//Flags
};
