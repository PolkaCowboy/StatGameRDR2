#include "script.h"
#include "keyboard.h"
#include "log.h"
#include "config.h"
#include "ui.h"
#include "challenges.h"
#include "rewards.h"

#include <format>
#include <vector>
#include <algorithm>
#include <random>

namespace
{
	bool g_enabled = false;
	ModConfig g_config;
	Rewards rewards;

	// Highest rank SET_ATTRIBUTE_BASE_RANK will accept - anything above clamps to this.
	constexpr int kMaxRank = 7;

	std::mt19937& Rng()
	{
		static std::mt19937 rng(std::random_device{}());
		return rng;
	}

	void SyncAndSavePlayerStats(Ped player) {
		g_config.HealthCoreLevel = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH);
		g_config.StaminaCoreLevel = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA);
		g_config.DeadEyeCoreLevel = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY);

		SavePlayerStats(g_config);
	}

	bool CheckForVictory(Ped player) {
		if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH) == kMaxRank &&
			ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA) == kMaxRank &&
			ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY) == kMaxRank) {
			UI::DisplaySubtitle(g_config.VictoryMessage.c_str());
			return true;
		}
		return false;
	}

	const char* GetAttributeFriendlyName(ePedAttribute a) {
		switch (a) {
		case PA_HEALTH: return "Health";
		case PA_STAMINA: return "Stamina";
		case PA_SPECIALABILITY: return "Dead Eye";
		}
		return "Unknown";
	}

	void Punish(Ped player, bool isMinorPunishment, int statRanksToRemove) {
		std::vector<ePedAttribute> reducedAttrs;

		for (int n = 0; n < statRanksToRemove; n++) {
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
				int pick = MISC::GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(eligableAttrs.size()));
				reducedAttrs.push_back(eligableAttrs[pick]);
				int newValue = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, eligableAttrs[pick]) - 1;
				ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, eligableAttrs[pick], newValue);
			}
		}

		if (!reducedAttrs.empty()) {
			SyncAndSavePlayerStats(player);

			std::vector<std::pair<ePedAttribute, int>> tally;

			for (ePedAttribute a : reducedAttrs) {
				bool seen = false;

				for (auto& entry : tally) {
					if (entry.first == a) {
						entry.second++;
						seen = true;
						break;
					}
				}

				if (!seen) {
					tally.push_back({ a, 1 });
				}
			}

			std::string message = isMinorPunishment ? "Minor Punish: " : "Major Punish: ";

			for (size_t n = 0; n < tally.size(); n++) {
				if (n > 0) {
					message += ", ";
				}

				message += std::format("{} reduced by {} to {}",
					GetAttributeFriendlyName(tally[n].first),
					tally[n].second,
					ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, tally[n].first));
			}

			Log::Write(message);
			UI::DisplaySubtitle(message.c_str());
		}
		else {
			UI::DisplaySubtitle("No eligiable stats to decrease. Wow.");
		}
	}

	std::string Boost(Ped player, bool isMinorUp) {
		std::vector<ePedAttribute> eligableAttrs;

		if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH) < (isMinorUp ? g_config.HealthCoreMajorBreakpoint : kMaxRank)) {
			eligableAttrs.push_back(PA_HEALTH);
		}
		if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA) < (isMinorUp ? g_config.StaminaCoreMajorBreakpoint : kMaxRank)) {
			eligableAttrs.push_back(PA_STAMINA);
		}
		if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY) < (isMinorUp ? g_config.DeadEyeCoreMajorBreakpoint : kMaxRank)) {
			eligableAttrs.push_back(PA_SPECIALABILITY);
		}
		if (!eligableAttrs.empty()) {
			int i = MISC::GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(eligableAttrs.size()));

			int newValue = ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, eligableAttrs[i]) + 1;
			ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, eligableAttrs[i], newValue);

			SyncAndSavePlayerStats(player);

			std::string message = std::format("{} Increase: {} increased to {}",
				isMinorUp ? "Minor" : "Major",
				GetAttributeFriendlyName(eligableAttrs[i]),
				ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, eligableAttrs[i]));

			Log::Write(message);
			if (!CheckForVictory(player)) {
				return message;
			}
		}
		else if (isMinorUp) {
			return "No eligiable stats for minor increase";
		}
		return "";
	}

	std::string RewardGiftMaybe(Ped player) {
		int totalMinorStatPoints =
			g_config.HealthCoreMajorBreakpoint
			+ g_config.StaminaCoreMajorBreakpoint
			+ g_config.DeadEyeCoreMajorBreakpoint;

		int totalPlayerMinorStatPoints =
			std::min(ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH), g_config.HealthCoreMajorBreakpoint)
			+ std::min(ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA), g_config.StaminaCoreMajorBreakpoint)
			+ std::min(ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY), g_config.DeadEyeCoreMajorBreakpoint);

		Log::Write(std::format("Player has {} of {} minor stat points", totalPlayerMinorStatPoints, totalMinorStatPoints));

		std::uniform_int_distribution<int> dist(1, totalMinorStatPoints);
		int roll = dist(Rng());

		Log::Write(std::format("Rolled {} <= {}", roll, totalPlayerMinorStatPoints));
		if (roll <= totalPlayerMinorStatPoints) {
			//Find rewards that haven't been handed out
			std::vector<size_t> eligible;
			for (size_t i = 0; i < rewards.rewards.size(); i++) {
				if (!rewards.rewards[i].rewarded && !rewards.rewards[i].rewardPool.empty()) {
					eligible.push_back(i);
				}
			}

			if (eligible.empty()) {
				Log::Write("No eligable rewards");
				return "";
			}

			//Pick a Reward Pool
			std::uniform_int_distribution<size_t> rewardDist(0, eligible.size() - 1);
			Reward& reward = rewards.rewards[eligible[rewardDist(Rng())]];

			//Select actual item from the pool.
			std::uniform_int_distribution<size_t> poolDist(0, reward.rewardPool.size() - 1);
			const std::string& rewardedItem = reward.rewardPool[poolDist(Rng())];

			Log::Write(std::format("Rewarding {}", rewardedItem));

			//Log it and ship it.
			reward.rewarded = true;
			reward.rewardedItem = rewardedItem;
			if (!SaveRewards(rewards)) {
				Log::Write("Failed to save rewards.json - this reward may be handed out again");
				return std::format(" \nYou've been rewarded \"{}\", (but the .json save failed)", rewardedItem);
			}
			return std::format(" \nYou've been rewarded \"{}\"", rewardedItem);
		}
		else {
			Log::Write("No Reward. Try again");
		}
		return "";
	}

	void OnTick()
	{
		Ped player = PLAYER::PLAYER_PED_ID();

		if (IsKeyJustUp(g_config.LoadCoreValues)) {
			g_config = LoadConfig();
			rewards = LoadRewards();

			Log::Write("Load key pressed");
			Log::Write(std::format("Health: {}", g_config.HealthCoreLevel));
			Log::Write(std::format("Stamina: {}", g_config.StaminaCoreLevel));
			Log::Write(std::format("Deadeye: {}", g_config.DeadEyeCoreLevel));

			ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, PA_HEALTH, g_config.HealthCoreLevel);
			ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, PA_STAMINA, g_config.StaminaCoreLevel);
			ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, PA_SPECIALABILITY, g_config.DeadEyeCoreLevel);

			UI::DisplaySubtitle(std::format("Player stats set to Health: {}, Stamina: {}, Deadeye: {}", g_config.HealthCoreLevel, g_config.StaminaCoreLevel, g_config.DeadEyeCoreLevel).c_str());
		}

		//  Minor Up. +1 random stat. Can't go past the major breakpoint.
		if (IsKeyJustUp(g_config.MinorUpKey)) {
			std::string message = Boost(player, true);
			message += RewardGiftMaybe(player);
			if (!message.empty()) {
				UI::DisplaySubtitle(message.c_str());
			}
		}

		// Major up. Can go all the way to the 8th tier. Can pick stats not yet at the breakpoint.
		if (IsKeyJustUp(g_config.MajorUpKey)) {
			std::string message = Boost(player, false);
			message += RewardGiftMaybe(player);
			if (!message.empty()) {
				UI::DisplaySubtitle(message.c_str());
			}
		}

		// Minor punish. 
		if (IsKeyJustUp(g_config.MinorPunishKey)) {
			Punish(player, true, g_config.LevelsLostOnMinorPunishment);
		}

		//Major punish.
		if (IsKeyJustUp(g_config.MajorPunishKey)) {
			Punish(player, false, g_config.LevelsLostOnMajorPunishment);
		}

	}
}

void ScriptMain()
{

	g_config = LoadConfig();
	rewards = LoadRewards();

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
