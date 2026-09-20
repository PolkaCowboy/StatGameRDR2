#pragma once

#include <windows.h>
#include <string>

// Minimal file logger. Writes log.log into the mod folder
namespace Log
{
	void Init();

	void Write(const std::string &message);
}
