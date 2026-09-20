#pragma once

#include <windows.h>
#include <string>

struct ModConfig
{

	//Core Levels
	int HealthCoreLevel = 2;
	int StaminaCoreLevel = 2;
	int DeadEyeCoreLevel = 2;

	//Major Core Breakpoints
	int HealthCoreMajorBreakpoint = 5;
	int StaminaCoreMajorBreakpoint = 5;
	int DeadEyeCoreMajorBreakpoint = 5;

	int LevelsLostOnMinorPunishment = 1;
	int LevelsLostOnMajorPunishment = 2;

	//Keys
	DWORD MinorUpKey = VK_F13;
	DWORD MajorUpKey = VK_F14;
	DWORD MinorPunishKey = VK_F15;
	DWORD MajorPunishKey = VK_F16;
	DWORD LoadCoreValues = VK_F17;

	DWORD toggleKey = VK_F19;

	bool enableTextLogging = true;

};

ModConfig LoadConfig();
