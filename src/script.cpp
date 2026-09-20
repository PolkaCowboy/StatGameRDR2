#include "script.h"
#include "keyboard.h"
#include "log.h"
#include "config.h"
#include "ui.h"
#include "challenges.h"

#include <format>
#include <vector>

namespace
{
	bool g_enabled = false;
	ModConfig g_config;

	void ChangeAndLogPlayerStat(Ped player, ePedAttribute attribute, int change) {
		ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, attribute, ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, attribute) + change);

		g_config.HealthCoreLevel = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH);
		g_config.StaminaCoreLevel = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA);
		g_config.DeadEyeCoreLevel = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY);

		SavePlayerStats(g_config);

		if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH) == 7 &&
			ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA) == 7 &&
			ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY) == 7) {
			UI::DisplaySubtitle("A winner is you!");
		}

	}

	const char* GetAttributeFriendlyName(ePedAttribute a){
		switch (a) {
			case PA_HEALTH: return "Health";
			case PA_STAMINA: return "Stamina";
			case PA_SPECIALABILITY: return "Dead Eye";
		}
	}

	void OnTick()
	{
		Ped player = PLAYER::PLAYER_PED_ID();

		//UI::DrawDebugText(std::format("Stamina points: {}", ATTRIBUTE::GET_ATTRIBUTE_POINTS(player, PA_STAMINA)).c_str());

		if (IsKeyJustUp(g_config.LoadCoreValues)) {
			g_config = LoadConfig();
			
			Log::Write("Load key pressed");
			Log::Write(std::format("Health: {0}", g_config.HealthCoreLevel));
			Log::Write(std::format("Stamina: {0}", g_config.StaminaCoreLevel));
			Log::Write(std::format("Deadeye: {0}", g_config.DeadEyeCoreLevel));

			ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH, g_config.HealthCoreLevel);
			ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA, g_config.StaminaCoreLevel);
			ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY, g_config.DeadEyeCoreLevel);

			UI::DisplaySubtitle(std::format("Player stats set to Health: {0}, Stamina: {1}, Deadeye: {2}", g_config.HealthCoreLevel, g_config.StaminaCoreLevel, g_config.DeadEyeCoreLevel).c_str());
		}

		//  Minor Up. +1 random stat. Can't go past the major breakpoint.
		if (IsKeyJustUp(g_config.MinorUpKey)) {
			
			Log::Write("Minor Up");

			std::vector<ePedAttribute> eligableAttrs;

			if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH) < g_config.HealthCoreMajorBreakpoint) {
				eligableAttrs.push_back(PA_HEALTH);
			}
			if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA) < g_config.StaminaCoreMajorBreakpoint) {
				eligableAttrs.push_back(PA_STAMINA);
			}
			if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY) < g_config.DeadEyeCoreMajorBreakpoint) {
				eligableAttrs.push_back(PA_SPECIALABILITY);
			}
			if (!eligableAttrs.empty()) {
				int i = MISC::GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(eligableAttrs.size()));
				Log::Write(std::format("{0} Chosen", GetAttributeFriendlyName(eligableAttrs[i])));
				UI::DisplaySubtitle(std::format("{0} Increased", GetAttributeFriendlyName(eligableAttrs[i])).c_str());
				ChangeAndLogPlayerStat(player, eligableAttrs[i], 1);
			}
			else {
				UI::DisplaySubtitle("No eligiable stats for minor increase");
			}

		}

		// Major up. Can go all the way to the 8th tier. Can pick stats not yet at the breakpoint.
		if (IsKeyJustUp(g_config.MajorUpKey)) {
			Log::Write("Major Up");

			std::vector<ePedAttribute> eligableAttrs;

			if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH) < 7) {
				eligableAttrs.push_back(PA_HEALTH);
			}
			if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA) < 7) {
				eligableAttrs.push_back(PA_STAMINA);
			}
			if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY) < 7) {
				eligableAttrs.push_back(PA_SPECIALABILITY);
			}

			if (!eligableAttrs.empty()) {
				int i = MISC::GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(eligableAttrs.size()));
				Log::Write(std::format("{0} Chosen", GetAttributeFriendlyName(eligableAttrs[i])));
				UI::DisplaySubtitle(std::format("{0} Increased", GetAttributeFriendlyName(eligableAttrs[i])).c_str());
				ChangeAndLogPlayerStat(player, eligableAttrs[i], 1);
			}

		}


	}
}

void ScriptMain()
{

	g_config = LoadConfig();

	Log::SetEnabled(g_config.enableTextLogging);
	Log::Init();
	Log::Write("ScriptMain started");
	UI::DisplaySubtitle("StatGame Enabled");

	while (true)
	{
		OnTick();
		WAIT(0);
	}
}
