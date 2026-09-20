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

	void ChangeAndLogPlayerStat(Ped player, ePedAttribute attribute, int newValue) {
		ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, attribute, newValue);

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

	const char* GetAttributeFriendlyName(ePedAttribute a) {
		switch (a) {
			case PA_HEALTH: return "Health";
			case PA_STAMINA: return "Stamina";
			case PA_SPECIALABILITY: return "Dead Eye";
		}
		return "Unknown";
	}

	void OnTick()
	{
		Ped player = PLAYER::PLAYER_PED_ID();

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

				int newValue = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, eligableAttrs[i]) + 1;
				std::string message = std::format("Minor Increase: {0} increased to {1}", GetAttributeFriendlyName(eligableAttrs[i]), newValue);

				Log::Write(message);
				UI::DisplaySubtitle(message.c_str());

				ChangeAndLogPlayerStat(player, eligableAttrs[i], newValue);
			}
			else {
				UI::DisplaySubtitle("No eligiable stats for minor increase");
			}

		}

		// Major up. Can go all the way to the 8th tier. Can pick stats not yet at the breakpoint.
		if (IsKeyJustUp(g_config.MajorUpKey)) {

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

				int newValue = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, eligableAttrs[i]) + 1;
				std::string message = std::format("Major Increase: {0} increased to {1}", GetAttributeFriendlyName(eligableAttrs[i]), newValue);

				Log::Write(message);
				UI::DisplaySubtitle(message.c_str());

				ChangeAndLogPlayerStat(player, eligableAttrs[i], newValue);
			}
		}

		// Minor down. Pick one stat and drop it by 1.
		if (IsKeyJustUp(g_config.MinorPunishKey)) {
			std::vector<ePedAttribute> eligableAttrs;

			if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH) > 0) {
				eligableAttrs.push_back(PA_HEALTH);
			}
			if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA) > 0) {
				eligableAttrs.push_back(PA_STAMINA);
			}
			if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY) > 0) {
				eligableAttrs.push_back(PA_SPECIALABILITY);
			}

			if (!eligableAttrs.empty()) {

				int i = MISC::GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(eligableAttrs.size()));
				int newValue = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, eligableAttrs[i]) - 1;
				std::string message = std::format("Minor Punishment: {0} decreased to {1}", GetAttributeFriendlyName(eligableAttrs[i]), newValue);

				Log::Write(message);
				UI::DisplaySubtitle(message.c_str());
				ChangeAndLogPlayerStat(player, eligableAttrs[i], newValue);
			}
			else {
				UI::DisplaySubtitle("No eligiable stats to decrease. Wow.");
			}
		}

		//Major punish. -2 random skills.
		if (IsKeyJustUp(g_config.MajorPunishKey)) {
			std::vector<ePedAttribute> reducedAttrs;
			for (int i = 0; i <= 1; i++) {
				std::vector<ePedAttribute> eligableAttrs;

				if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH) > 0) {
					eligableAttrs.push_back(PA_HEALTH);
				}
				if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA) > 0) {
					eligableAttrs.push_back(PA_STAMINA);
				}
				if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY) > 0) {
					eligableAttrs.push_back(PA_SPECIALABILITY);
				}

				if (!eligableAttrs.empty()) {
					int i = MISC::GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(eligableAttrs.size()));
					reducedAttrs.push_back(eligableAttrs[i]);
					int newValue = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, eligableAttrs[i]) - 1;
					ChangeAndLogPlayerStat(player, eligableAttrs[i], newValue);
				}
			}

			if (!reducedAttrs.empty()) {
				std::string message;

				if (reducedAttrs.size() == 2 && reducedAttrs[0] == reducedAttrs[1]) {
					message = std::format("Major Punish: {0} reduced by 2 to {1}",
						GetAttributeFriendlyName(reducedAttrs[0]),
						ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, reducedAttrs[0]));
				}
				else {
					message = "Major Punish: ";

					for (size_t n = 0; n < reducedAttrs.size(); n++) {
						if (n > 0) {
							message += ", ";
						}

						message += std::format("{0} reduced to {1}",
							GetAttributeFriendlyName(reducedAttrs[n]),
							ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, reducedAttrs[n]));
					}
				}

				Log::Write(message);
				UI::DisplaySubtitle(message.c_str());
			}
			else {
				UI::DisplaySubtitle("No eligiable stats to decrease. Wow.");
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
