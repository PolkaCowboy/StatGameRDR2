#include "config.h"
#include "paths.h"
#include "../inc/SimpleIni.h"

#include <filesystem>
#include <format>

namespace fs = std::filesystem;

ModConfig LoadConfig()
{
	ModConfig cfg; // defaults from config.h

	const std::string path = (fs::path(ModDataDir()) / "config.ini").string();

	CSimpleIniA ini;
	ini.SetUnicode();

	if (!fs::exists(path))
	{
		// First run: write the defaults out so there is something to edit.
		ini.SetValue("Cores", nullptr, nullptr, 
			"; Player core levels. The player's current core level will be written to these when the Load hotkey is used.\n"
			"; These values will auto update in the.ini as the game progresses.\n"
			"; Restore them to your preferred starting value to start a new game.\n"
			"; IMPORTANT: The game currently only supports the first 8 levels (0-7).\n"
			"; I haven't been able to figure out how to get around the challenge unlock prereqs for final two tiers"
		);
		ini.SetLongValue("Cores", "Health", cfg.HealthCoreLevel);
		ini.SetLongValue("Cores", "Stamina", cfg.StaminaCoreLevel);
		ini.SetLongValue("Cores", "DeadEye", cfg.DeadEyeCoreLevel);

		ini.SetValue("Breakpoints", nullptr, nullptr,
			"; Stat level that requires a Major Level Up to proceed past");
		ini.SetLongValue("Breakpoints", "Health", cfg.HealthCoreMajorBreakpoint);
		ini.SetLongValue("Breakpoints", "Stamina", cfg.StaminaCoreMajorBreakpoint);
		ini.SetLongValue("Breakpoints", "DeadEye", cfg.DeadEyeCoreMajorBreakpoint);

		ini.SetLongValue("General", "Levels lost on minor punishment", cfg.LevelsLostOnMinorPunishment);
		ini.SetLongValue("General", "Levels lost on major punishment", cfg.LevelsLostOnMajorPunishment);

		ini.SetValue("Keys", nullptr, nullptr,
			"; Virtual-key codes in hex. 0x82 = F19.\n"
			"; Full list: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes");
		ini.SetLongValue("Keys", "Minor Up", cfg.MinorUpKey, "; F13", true);
		ini.SetLongValue("Keys", "Major Up", cfg.MajorUpKey, "; F14", true);
		ini.SetLongValue("Keys", "Minor Punish", cfg.MinorPunishKey, "; F15", true);
		ini.SetLongValue("Keys", "Major Punish", cfg.MajorPunishKey, "; F16", true);
		ini.SetLongValue("Keys", "Load Cores From File", cfg.LoadCoreValues, "; F17", true);

		ini.SaveFile(path.c_str(), false);
		return cfg;
	}

	// Exists but unreadable: use the defaults rather than clobbering the file.
	if (ini.LoadFile(path.c_str()) < 0)
		return cfg;

	cfg.HealthCoreLevel = static_cast<int>(ini.GetLongValue("Cores", "Health", cfg.HealthCoreLevel));
	cfg.StaminaCoreLevel = static_cast<int>(ini.GetLongValue("Cores", "Stamina", cfg.StaminaCoreLevel));
	cfg.DeadEyeCoreLevel = static_cast<int>(ini.GetLongValue("Cores", "DeadEye", cfg.DeadEyeCoreLevel));

	cfg.HealthCoreMajorBreakpoint = static_cast<int>(ini.GetLongValue("Breakpoints", "Health", cfg.HealthCoreMajorBreakpoint));
	cfg.StaminaCoreMajorBreakpoint = static_cast<int>(ini.GetLongValue("Breakpoints", "Stamina", cfg.StaminaCoreMajorBreakpoint));
	cfg.DeadEyeCoreMajorBreakpoint = static_cast<int>(ini.GetLongValue("Breakpoints", "DeadEye", cfg.DeadEyeCoreMajorBreakpoint));

	cfg.LevelsLostOnMinorPunishment = static_cast<int>(ini.GetLongValue("General", "Levels lost on minor punishment", cfg.LevelsLostOnMinorPunishment));
	cfg.LevelsLostOnMajorPunishment = static_cast<int>(ini.GetLongValue("General", "Levels lost on major punishment", cfg.LevelsLostOnMajorPunishment));

	cfg.MinorUpKey = static_cast<DWORD>(ini.GetLongValue("Keys", "Minor Up", cfg.MinorUpKey));
	cfg.MajorUpKey = static_cast<DWORD>(ini.GetLongValue("Keys", "Major Up", cfg.MajorUpKey));
	cfg.MinorPunishKey = static_cast<DWORD>(ini.GetLongValue("Keys", "Minor Punish", cfg.MinorPunishKey));
	cfg.MajorPunishKey = static_cast<DWORD>(ini.GetLongValue("Keys", "Major Punish", cfg.MajorPunishKey));
	cfg.LoadCoreValues = static_cast<DWORD>(ini.GetLongValue("Keys", "Load Cores From File", cfg.LoadCoreValues));

	cfg.toggleKey = static_cast<DWORD>(ini.GetLongValue("Keys", "ToggleKey", cfg.toggleKey));
	cfg.enableTextLogging = ini.GetBoolValue("Debug", "EnableTextLogging", cfg.enableTextLogging);

	return cfg;
}
