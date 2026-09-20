#include "log.h"
#include "paths.h"

#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>

namespace fs = std::filesystem;

namespace
{
	std::string g_logPath;
}

void Log::Init()
{
	g_logPath = (fs::path(ModDataDir()) / "log.log").string();

	std::ofstream(g_logPath, std::ios::trunc);
}

void Log::Write(const std::string &message)
{
	if (g_logPath.empty())
		return;

	std::ofstream file(g_logPath, std::ios::app);
	if (!file)
		return;

	auto t = std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now());
	std::string s = std::format("{:%T}", std::chrono::zoned_time{ std::chrono::current_zone(), t });

	file << "[" << s << "] " << message << "\n";
}
