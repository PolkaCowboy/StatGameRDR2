#pragma once

#include <windows.h>
#include <string>

// Minimal file logger. Writes log.log into the mod folder
namespace Log
{
	void Init();
	void SetEnabled(bool enabled);
	void Write(const std::string &message);
}
