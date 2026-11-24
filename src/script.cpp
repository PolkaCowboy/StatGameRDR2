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

	// Set once the levels are loaded or changed; from then on the game's ranks are held on them.
	bool g_holdLevels = false;

	std::mt19937& Rng()
	{
		static std::mt19937 rng(std::random_device{}());
		return rng;
	}

	// StatGame's levels are the record. The game's ranks only follow them.
	int& CoreLevel(ePedAttribute a) {
		switch (a) {
		case PA_STAMINA: return g_config.StaminaCoreLevel;
		case PA_SPECIALABILITY: return g_config.DeadEyeCoreLevel;
		default: return g_config.HealthCoreLevel;
		}
	}

	// The game levels cores from XP on its own (running earns stamina XP) and works the rank out again
	// from the XP total, undoing SET_ATTRIBUTE_BASE_RANK. So the XP is held at the start of each level,
	// which is how Rockstar sets a horse's bonding level (abigail2_1.c func_1787).
	// https://github.com/JayKoZa/RDR2-Decompiled-Scripts/blob/master/script_rel/abigail2_1.c
	void ApplyLevels(Ped player) {
		Hash model = ENTITY::GET_ENTITY_MODEL(player);
		for (ePedAttribute a : { PA_HEALTH, PA_STAMINA, PA_SPECIALABILITY }) {
			int level = CoreLevel(a);
			int points = ATTRIBUTE::GET_DEFAULT_ATTRIBUTE_POINTS_NEEDED_FOR_RANK(model, a, level);
			if (ATTRIBUTE::GET_ATTRIBUTE_POINTS(player, a) != points) {
				ATTRIBUTE::SET_ATTRIBUTE_POINTS(player, a, points);
			}
			if (ATTRIBUTE::GET_ATTRIBUTE_BASE_RANK(player, a) != level) {
				ATTRIBUTE::SET_ATTRIBUTE_BASE_RANK(player, a, level);
			}
		}
	}

	void ApplyAndSaveLevels(Ped player) {
		g_holdLevels = true;
		ApplyLevels(player);
		SavePlayerStats(g_config);
	}

	bool CheckForVictory() {
		if (CoreLevel(PA_HEALTH) == kMaxRank &&
			CoreLevel(PA_STAMINA) == kMaxRank &&
			CoreLevel(PA_SPECIALABILITY) == kMaxRank) {
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

			if (CoreLevel(PA_HEALTH) > 0) {
				eligableAttrs.push_back(PA_HEALTH);
			}
			if (CoreLevel(PA_STAMINA) > 0) {
				eligableAttrs.push_back(PA_STAMINA);
			}
			if (CoreLevel(PA_SPECIALABILITY) > 0) {
				eligableAttrs.push_back(PA_SPECIALABILITY);
			}

			if (!eligableAttrs.empty()) {
				int pick = MISC::GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(eligableAttrs.size()));
				reducedAttrs.push_back(eligableAttrs[pick]);
				CoreLevel(eligableAttrs[pick])--;
			}
		}

		if (!reducedAttrs.empty()) {
			ApplyAndSaveLevels(player);

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
					CoreLevel(tally[n].first));
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

		if (CoreLevel(PA_HEALTH) < (isMinorUp ? g_config.HealthCoreMajorBreakpoint : kMaxRank)) {
			eligableAttrs.push_back(PA_HEALTH);
		}
		if (CoreLevel(PA_STAMINA) < (isMinorUp ? g_config.StaminaCoreMajorBreakpoint : kMaxRank)) {
			eligableAttrs.push_back(PA_STAMINA);
		}
		if (CoreLevel(PA_SPECIALABILITY) < (isMinorUp ? g_config.DeadEyeCoreMajorBreakpoint : kMaxRank)) {
			eligableAttrs.push_back(PA_SPECIALABILITY);
		}
		if (!eligableAttrs.empty()) {
			int i = MISC::GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(eligableAttrs.size()));

			CoreLevel(eligableAttrs[i])++;
			ApplyAndSaveLevels(player);

			std::string message = std::format("{} Increase: {} increased to {}",
				isMinorUp ? "Minor" : "Major",
				GetAttributeFriendlyName(eligableAttrs[i]),
				CoreLevel(eligableAttrs[i]));

			Log::Write(message);
			if (!CheckForVictory()) {
				return message;
			}
		}
		else if (isMinorUp) {
			return "No eligiable stats for minor increase";
		}
		return "";
	}

	std::string RewardGiftMaybe() {
		int totalMinorStatPoints =
			g_config.HealthCoreMajorBreakpoint
			+ g_config.StaminaCoreMajorBreakpoint
			+ g_config.DeadEyeCoreMajorBreakpoint;

		int totalPlayerMinorStatPoints =
			std::min(CoreLevel(PA_HEALTH), g_config.HealthCoreMajorBreakpoint)
			+ std::min(CoreLevel(PA_STAMINA), g_config.StaminaCoreMajorBreakpoint)
			+ std::min(CoreLevel(PA_SPECIALABILITY), g_config.DeadEyeCoreMajorBreakpoint);

		Log::Write(std::format("Player has {} of {} minor stat points", totalPlayerMinorStatPoints, totalMinorStatPoints));

		std::uniform_int_distribution<int> dist(1, totalMinorStatPoints);
		int roll = dist(Rng());

		Log::Write(std::format("Rolled {} <= {}", roll, totalPlayerMinorStatPoints));
		if (roll <= totalPlayerMinorStatPoints) {
			//Find rewards that haven't been handed out
			std::vector<size_t> eligible;
			std::vector<float> weights;
			for (size_t i = 0; i < rewards.rewards.size(); i++) {
				if (!rewards.rewards[i].rewarded && !rewards.rewards[i].rewardPool.empty() && rewards.rewards[i].weight > 0.0f) {
					eligible.push_back(i);
					weights.push_back(rewards.rewards[i].weight);
				}
			}

			if (eligible.empty()) {
				Log::Write("No eligable rewards");
				return "";
			}

			//Pick a Reward Pool
			std::discrete_distribution<size_t> rewardDist(weights.begin(), weights.end());
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

		if (g_holdLevels && ENTITY::DOES_ENTITY_EXIST(player)) {
			ApplyLevels(player);
		}

		if (IsKeyJustUp(g_config.LoadCoreValues)) {
			g_config = LoadConfig();
			rewards = LoadRewards();

			Log::Write("Load key pressed");
			Log::Write(std::format("Health: {}", g_config.HealthCoreLevel));
			Log::Write(std::format("Stamina: {}", g_config.StaminaCoreLevel));
			Log::Write(std::format("Deadeye: {}", g_config.DeadEyeCoreLevel));

			g_holdLevels = true;
			ApplyLevels(player);

			UI::DisplaySubtitle(std::format("Player stats set to Health: {}, Stamina: {}, Deadeye: {}", g_config.HealthCoreLevel, g_config.StaminaCoreLevel, g_config.DeadEyeCoreLevel).c_str());
		}

		//  Minor Up. +1 random stat. Can't go past the major breakpoint.
		if (IsKeyJustUp(g_config.MinorUpKey)) {
			std::string message = Boost(player, true);
			message += RewardGiftMaybe();
			if (!message.empty()) {
				UI::DisplaySubtitle(message.c_str());
			}
		}

		// Major up. Can go all the way to the 8th tier. Can pick stats not yet at the breakpoint.
		if (IsKeyJustUp(g_config.MajorUpKey)) {
			std::string message = Boost(player, false);
			message += RewardGiftMaybe();
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
