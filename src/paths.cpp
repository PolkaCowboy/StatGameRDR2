#include "paths.h"

#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace
{
	constexpr const char* ROOT_MOD_FOLDER = "PolkaMods";
	constexpr const char* THIS_MOD_FOLDER = "StatGame";
}

// Absolute path to this mod's data folder, beside RDR2.exe:
//   <game>\PolkaMods\<ThisMod>
//
// Created on first call if it does not exist.
const std::string& ModDataDir()
{
	static const std::string dir = []() -> std::string {
		char exePath[MAX_PATH]{};
		if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) == 0)
			return ".";

		fs::path p = fs::path(exePath).parent_path() / ROOT_MOD_FOLDER / THIS_MOD_FOLDER;

		// The error_code overload does not throw. Never let an exception escape
		// into the game.
		std::error_code ec;
		fs::create_directories(p, ec);
		return p.string();
	}();

	return dir;
}
