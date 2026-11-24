#pragma once

#include "../inc/nlohmann/json.hpp"

#include <string>
#include <vector>

struct Reward
{
	bool rewarded = false;
	float weight = 1.0f;
	std::vector<std::string> rewardPool;
	std::string rewardedItem = "";
};

struct Rewards
{
	std::vector<Reward> rewards;
};

// Generates the JSON conversions from the member list, so adding a field above
// picks it up automatically. _WITH_DEFAULT leaves the struct initialiser alone
// when a key is missing instead of throwing.
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Reward, rewarded, weight, rewardPool, rewardedItem)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Rewards, rewards)

Rewards LoadRewards();
bool SaveRewards(const Rewards& rewards);
