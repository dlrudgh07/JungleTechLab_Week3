#include "IniConfig.h"

FIniConfig::FIniConfig()
{
	IniParser.ReadFileToString(INI_PATH);
	SetGridOffset(IniParser.GetValue("Grid", "Offset", GridOffset, 1.0f));
	SetGridRange(IniParser.GetValue("Grid", "Range", GridRange, 100));
	SetCameraSensitivity(IniParser.GetValue("Camera", "Sensitivity", CameraSensitivity, 0.1f));
	SetCameraSpeed(IniParser.GetValue("Camera", "Speed", CameraSpeed, 5.0f));
}

float FIniConfig::GetGridOffset()
{
	return (GridOffset);
}


int32 FIniConfig::GetGridRange()
{
	return (GridRange);
}

float FIniConfig::GetCameraSensitivity()
{
	return (CameraSensitivity);
}

float FIniConfig::GetCameraSpeed()
{
	return (CameraSpeed);
}

void FIniConfig::SetGridOffset(float _Offset)
{
	GridOffset = std::clamp(_Offset, 0.1f, 20.0f);
}

void FIniConfig::SetGridRange(int32 _Range)
{
	GridRange = std::clamp(_Range, 1, 100);
}

void FIniConfig::SetCameraSensitivity(float _Sensitivity)
{
	CameraSensitivity = std::clamp(_Sensitivity, 0.01f, 0.5f);
}

void FIniConfig::SetCameraSpeed(float _Speed)
{
	CameraSpeed = std::clamp(_Speed, 0.1f, 100.0f);
}

void FIniConfig::Save()
{
	IniParser.SetValue("Grid", "Offset", GridOffset);
	IniParser.SetValue("Grid", "Range", GridRange);
	IniParser.SetValue("Camera", "Sensitivity", CameraSensitivity);
	IniParser.SetValue("Camera", "Speed", CameraSpeed);
	IniParser.WriteStringToFile(INI_PATH);

}
