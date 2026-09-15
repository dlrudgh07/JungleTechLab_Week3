#pragma once

#include "IniParser.h"
#include "Enum.h"

#define INI_PATH "ini\\editor.ini"


class FIniConfig
{
public:
	FIniConfig();

	float	GetGridOffset();
	int32	GetGridRange();

	float	GetCameraSensitivity();
	float	GetCameraSpeed();

	void	LoadShowFlags();
	void	SaveShowFlags();

	EEngineShowFlags GetShowFlags() const;
	
	void	SetGridOffset(float _Offset);	
	void	SetGridRange(int32 _Range);	
	void	SetCameraSensitivity(float _Sensitivity);
	void	SetCameraSpeed(float _Speed);

	void SetShowFlag(EEngineShowFlags showflag, bool bEnabled); //1이면 스위치켜기, 0이면 끄기

	void	Save();

private:
	FIniParser IniParser;
	//Grid
	float	GridOffset;
	int32	GridRange;
	//Camera
	float	CameraSensitivity;
	float	CameraSpeed;
	//Flag
	int	ShowFlag;

	EEngineShowFlags ShowFlags;

};
