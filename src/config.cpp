#include "config.h"
#include "paths.h"
#include "../inc/SimpleIni.h"

#include <filesystem>
#include <format>

namespace fs = std::filesystem;

namespace
{
	const std::string& ConfigPath()
	{
		static const std::string path = (fs::path(ModDataDir()) / "config.ini").string();
		return path;
	}
}

ModConfig LoadConfig()
{
	ModConfig cfg; // defaults from config.h

	CSimpleIniA ini;
	ini.SetUnicode();

	if (!fs::exists(ConfigPath()))
	{
		// First run: write the defaults out so there is something to edit.
		ini.SetValue("CORES", nullptr, nullptr, 
			"; Player core levels. The player's current core level will be written to these when the Load hotkey is used.\n"
			"; These values will auto update in the.ini as the game progresses.\n"
			"; Restore them to your preferred starting value to start a new game.\n"
			"; IMPORTANT: The game currently only supports the first 8 levels (0-7).\n"
			"; I haven't been able to figure out how to get around the challenge unlock prereqs for final two tiers"
		);
		ini.SetLongValue("CORES", "HEALTH", cfg.HealthCoreLevel);
		ini.SetLongValue("CORES", "STAMINA", cfg.StaminaCoreLevel);
		ini.SetLongValue("CORES", "DEAD_EYE", cfg.DeadEyeCoreLevel);

		ini.SetValue("BREAKPOINTS", nullptr, nullptr,
			"; Stat level that requires a Major Level Up to proceed past");
		ini.SetLongValue("BREAKPOINTS", "HEALTH", cfg.HealthCoreMajorBreakpoint);
		ini.SetLongValue("BREAKPOINTS", "STAMINA", cfg.StaminaCoreMajorBreakpoint);
		ini.SetLongValue("BREAKPOINTS", "DEAD_EYE", cfg.DeadEyeCoreMajorBreakpoint);

		ini.SetLongValue("GENERAL", "LEVELS_LOST_ON_MINOR_PUNISHMENT", cfg.LevelsLostOnMinorPunishment);
		ini.SetLongValue("GENERAL", "LEVELS_LOST_ON_MAJOR_PUNISHMENT", cfg.LevelsLostOnMajorPunishment);
		ini.SetValue("GENERAL", "VICTORY_MESSAGE", cfg.VictoryMessage.c_str());

		ini.SetValue("KEYS", nullptr, nullptr,
			"; Virtual-key codes in hex. 0x82 = F19.\n"
			"; Full list: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes");
		ini.SetLongValue("KEYS", "MINOR_UP", cfg.MinorUpKey, "; F13", true);
		ini.SetLongValue("KEYS", "MAJOR_UP", cfg.MajorUpKey, "; F14", true);
		ini.SetLongValue("KEYS", "MINOR_PUNISH", cfg.MinorPunishKey, "; F15", true);
		ini.SetLongValue("KEYS", "MAJOR_PUNISH", cfg.MajorPunishKey, "; F16", true);
		ini.SetLongValue("KEYS", "LOAD_CORES_FROM_FILE", cfg.LoadCoreValues, "; F17", true);

		ini.SaveFile(ConfigPath().c_str(), false);
		return cfg;
	}

	// Exists but unreadable: use the defaults rather than clobbering the file.
	if (ini.LoadFile(ConfigPath().c_str()) < 0)
		return cfg;

	cfg.HealthCoreLevel = static_cast<int>(ini.GetLongValue("CORES", "HEALTH", cfg.HealthCoreLevel));
	cfg.StaminaCoreLevel = static_cast<int>(ini.GetLongValue("CORES", "STAMINA", cfg.StaminaCoreLevel));
	cfg.DeadEyeCoreLevel = static_cast<int>(ini.GetLongValue("CORES", "DEAD_EYE", cfg.DeadEyeCoreLevel));

	cfg.HealthCoreMajorBreakpoint = static_cast<int>(ini.GetLongValue("BREAKPOINTS", "HEALTH", cfg.HealthCoreMajorBreakpoint));
	cfg.StaminaCoreMajorBreakpoint = static_cast<int>(ini.GetLongValue("BREAKPOINTS", "STAMINA", cfg.StaminaCoreMajorBreakpoint));
	cfg.DeadEyeCoreMajorBreakpoint = static_cast<int>(ini.GetLongValue("BREAKPOINTS", "DEAD_EYE", cfg.DeadEyeCoreMajorBreakpoint));

	cfg.LevelsLostOnMinorPunishment = static_cast<int>(ini.GetLongValue("GENERAL", "LEVELS_LOST_ON_MINOR_PUNISHMENT", cfg.LevelsLostOnMinorPunishment));
	cfg.LevelsLostOnMajorPunishment = static_cast<int>(ini.GetLongValue("GENERAL", "LEVELS_LOST_ON_MAJOR_PUNISHMENT", cfg.LevelsLostOnMajorPunishment));
	cfg.VictoryMessage = ini.GetValue("GENERAL", "VICTORY_MESSAGE", cfg.VictoryMessage.c_str());

	cfg.MinorUpKey = static_cast<DWORD>(ini.GetLongValue("KEYS", "MINOR_UP", cfg.MinorUpKey));
	cfg.MajorUpKey = static_cast<DWORD>(ini.GetLongValue("KEYS", "MAJOR_UP", cfg.MajorUpKey));
	cfg.MinorPunishKey = static_cast<DWORD>(ini.GetLongValue("KEYS", "MINOR_PUNISH", cfg.MinorPunishKey));
	cfg.MajorPunishKey = static_cast<DWORD>(ini.GetLongValue("KEYS", "MAJOR_PUNISH", cfg.MajorPunishKey));
	cfg.LoadCoreValues = static_cast<DWORD>(ini.GetLongValue("KEYS", "LOAD_CORES_FROM_FILE", cfg.LoadCoreValues));

	cfg.toggleKey = static_cast<DWORD>(ini.GetLongValue("KEYS", "TOGGLE_KEY", cfg.toggleKey));
	cfg.enableTextLogging = ini.GetBoolValue("DEBUG", "ENABLE_TEXT_LOGGING", cfg.enableTextLogging);

	return cfg;
}

void SavePlayerStats(const ModConfig& cfg)
{

	CSimpleIniA ini;
	ini.SetUnicode();

	if (ini.LoadFile(ConfigPath().c_str()) < 0)
		return;

	ini.SetLongValue("CORES", "HEALTH", cfg.HealthCoreLevel);
	ini.SetLongValue("CORES", "STAMINA", cfg.StaminaCoreLevel);
	ini.SetLongValue("CORES", "DEAD_EYE", cfg.DeadEyeCoreLevel);

	ini.SaveFile(ConfigPath().c_str(), false);
}
