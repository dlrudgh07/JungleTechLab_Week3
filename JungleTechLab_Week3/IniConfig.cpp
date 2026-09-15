#include "IniConfig.h"


FString BooleanToString(bool value)
{
	if (value)
		return (FString("true"));
	return (FString("false"));
}

FIniConfig::FIniConfig()
{
	IniParser.ReadFileToString(INI_PATH);
	SetGridOffset(IniParser.GetValue("Grid", "Offset", GridOffset, 1.0f));
	SetGridRange(IniParser.GetValue("Grid", "Range", GridRange, 100));

	SetCameraSensitivity(IniParser.GetValue("Camera", "Sensitivity", CameraSensitivity, 0.1f));
	SetCameraSpeed(IniParser.GetValue("Camera", "Speed", CameraSpeed, 5.0f));

	LoadShowFlags();
}

void FIniConfig::LoadShowFlags()
{
	if (IniParser.GetValue("Flag", "WorldAxis", true))
		ShowFlags |= (EEngineShowFlags::SF_WorldAxis);

	if (IniParser.GetValue("Flag", "ShowPrinmitives", true))
		ShowFlags |= (EEngineShowFlags::SF_Primitives);

	if (IniParser.GetValue("Flag", "ShowAABB", true))
		ShowFlags |= (EEngineShowFlags::SF_BoundingBoxes);

	if (IniParser.GetValue("Flag", "Gizmo", true))
		ShowFlags |= (EEngineShowFlags::SF_Gizmo);

	if (IniParser.GetValue("Flag", "BillBoard", true))
		ShowFlags |= (EEngineShowFlags::SF_BillboardText);

	if (IniParser.GetValue("Flag", "Grid", true))
		ShowFlags |= (EEngineShowFlags::SF_Grid);

	if (IniParser.GetValue("Flag", "UUID", true))
		ShowFlags |= (EEngineShowFlags::SF_UUID);

}

void FIniConfig::SaveShowFlags()
{
	IniParser.SetStringValue("Flag", "WorldAxis",
		BooleanToString(HasFlag(ShowFlags, EEngineShowFlags::SF_WorldAxis)));

	IniParser.SetStringValue("Flag", "ShowPrinmitives",
		BooleanToString(HasFlag(ShowFlags, EEngineShowFlags::SF_Primitives)));

	IniParser.SetStringValue("Flag", "ShowAABB",
		BooleanToString(HasFlag(ShowFlags, EEngineShowFlags::SF_BoundingBoxes)));

	IniParser.SetStringValue("Flag", "Gizmo",
		BooleanToString(HasFlag(ShowFlags, EEngineShowFlags::SF_Gizmo)));

	IniParser.SetStringValue("Flag", "BillBoard",
		BooleanToString(HasFlag(ShowFlags, EEngineShowFlags::SF_BillboardText)));

	IniParser.SetStringValue("Flag", "Grid",
		BooleanToString(HasFlag(ShowFlags, EEngineShowFlags::SF_Grid)));

	IniParser.SetStringValue("Flag", "UUID",
		BooleanToString(HasFlag(ShowFlags, EEngineShowFlags::SF_UUID)));
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

EEngineShowFlags FIniConfig::GetShowFlags() const
{
	return ShowFlags;
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

void FIniConfig::SetShowFlag(EEngineShowFlags showflag, bool bEnabled) //1이면 스위치켜기, 0이면 끄기
{
	if (bEnabled)
	{
		ShowFlags |= showflag;
	}

	else
	{
		ShowFlags &= ~showflag;
	}
}


void FIniConfig::Save()
{
	IniParser.SetValue("Grid", "Offset", GridOffset);
	IniParser.SetValue("Grid", "Range", GridRange);

	IniParser.SetValue("Camera", "Sensitivity", CameraSensitivity);
	IniParser.SetValue("Camera", "Speed", CameraSpeed);

	SaveShowFlags();

	IniParser.WriteStringToFile(INI_PATH);
}
