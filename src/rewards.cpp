#include "rewards.h"
#include "paths.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
	const std::string& RewardsPath()
	{
		static const std::string path = (fs::path(ModDataDir()) / "rewards.json").string();
		return path;
	}
}

Rewards LoadRewards() {

	Rewards rewards;

	std::ifstream file(RewardsPath());
	if (!file) {
		return rewards;
	}

	nlohmann::json j = nlohmann::json::parse(file, nullptr, false);

	if (j.is_discarded())
		return rewards;

	rewards = j.get<Rewards>();

	return rewards;

}

bool SaveRewards(const Rewards& rewards) {
	std::ofstream file(RewardsPath());
	if (!file)
		return false;

	file << nlohmann::json(rewards).dump(2); // 2 = indent, otherwise it's one unreadable line

	file.close();
	return file.good();  
}