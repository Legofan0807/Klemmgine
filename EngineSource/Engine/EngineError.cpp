#include "EngineError.h"
#include <Engine/Log.h>
#include <stacktrace>
#include <iostream>

void Engine::AssertFailure(std::string Name, std::string Location)
{
	Log::Print("[Error]: --------------------------------[Assert failed]--------------------------------", Log::LogColor::Red);
	Log::PrintMultiLine(Name, Log::LogColor::Red, "[Error]: ");
	Log::Print("[Error]: " + Location, Log::LogColor::Red);

#if __cpp_lib_stacktrace >= 202011L
	Log::Print("[Error]: ---------------------------------[Stack trace]---------------------------------", Log::LogColor::Red);
	auto trace = std::stacktrace::current();
	for (const auto& i : trace)
	{
#ifdef _MSC_VER
		std::string descr = i.description();
		descr = descr.substr(descr.find_first_of("!") + 1);
		descr = descr.substr(0, descr.find_first_of("+"));
		descr.append(", Line " + std::to_string(i.source_line()));
		if (i.source_file() == __FILE__)
		{
			continue;
		}
#else
		std::string descr = i.description();
#endif
		if (i.source_file().empty())
		{
			continue;
		}

		Log::Print("[Error]:\tat: " + FileUtil::GetFileNameFromPath(i.source_file()) + ": " + descr, Log::LogColor::Red);
	}
#endif

#if _WIN32
	__debugbreak();
#else
	throw 0;
#endif
}
