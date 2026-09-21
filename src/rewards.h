#pragma once

#include "../inc/nlohmann/json.hpp"

#include <string>
#include <vector>

struct Reward
{
	bool rewarded = false;
	std::vector<std::string> rewardPool;
};

struct Rewards
{
	std::vector<Reward> major;
	std::vector<Reward> minor;
};

// Generates the JSON conversions from the member list, so adding a field above
// picks it up automatically. _WITH_DEFAULT leaves the struct initialiser alone
// when a key is missing instead of throwing.
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Reward, rewarded, rewardPool)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Rewards, major, minor)

Rewards LoadRewards();
void SaveRewards(const Rewards& rewards);
