#include "script.h"
#include "keyboard.h"
#include "log.h"
#include "config.h"
#include "ui.h"
#include "challenges.h"

#include <format>

namespace
{
	bool g_enabled = false;
	ModConfig g_config;


	void OnTick()
	{
		Ped player = PLAYER::PLAYER_PED_ID();

		//UI::DrawDebugText(std::format("Stamina points: {}", ATTRIBUTE::GET_ATTRIBUTE_POINTS(player, PA_STAMINA)).c_str());

		if (IsKeyJustUp(g_config.LoadCoreValues)) {
			if (g_config.enableTextLogging) {
				Log::Write("Load key pressed");
				Log::Write(std::format("Health: {0}", g_config.HealthCoreLevel));
				Log::Write(std::format("Stamina: {0}", g_config.StaminaCoreLevel));
				Log::Write(std::format("Deadeye: {0}", g_config.DeadEyeCoreLevel));
			}

			ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH, g_config.HealthCoreLevel);
			ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA, g_config.StaminaCoreLevel);
			ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY, g_config.DeadEyeCoreLevel);

			UI::DisplaySubtitle(std::format("Player stats set to Health: {0}, Stamina: {1}, Deadeye: {2}", g_config.HealthCoreLevel, g_config.StaminaCoreLevel, g_config.DeadEyeCoreLevel).c_str());
		}

		if (IsKeyJustUp(g_config.MinorUpKey)) {
			if (g_config.enableTextLogging) {
				Log::Write("Minor Up");

			}
		}

	}
}

void ScriptMain()
{

	g_config = LoadConfig();

	if (g_config.enableTextLogging) {
		Log::Init();
		Log::Write("ScriptMain started");
		UI::DisplaySubtitle("StatGame Enabled");
	}

	while (true)
	{
		OnTick();
		WAIT(0);
	}
}
