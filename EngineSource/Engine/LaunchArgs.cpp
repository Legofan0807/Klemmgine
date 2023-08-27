#include <Engine/Console.h>
#include <vector>
#include <string>
#include <map>
#include <Engine/Log.h>
#include <Engine/OS.h>
#include <iostream>
#include <Engine/Scene.h>
#include <Rendering/Graphics.h>
#include <Engine/Application.h>

namespace LaunchArgs
{
	void NeverHideConsole(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size())
		{
			Log::Print("Unexpected arguments in -neverhideconsole", Log::LogColor::Yellow);
		}
		OS::SetConsoleCanBeHidden(false);
	}

	void LoadScene(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size() != 1)
		{
			Log::Print("Unexpected arguments in -loadscene", Log::LogColor::Yellow);
		}
		Application::StartupSceneOverride = AdditionalArgs[0];
	}

	void NoVSync(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size())
		{
			Log::Print("Unexpected arguments in -novsync", Log::LogColor::Yellow);
		}
		Graphics::VSync = false;
	}

	void Wireframe(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -wireframe", Log::LogColor::Yellow);
		Graphics::IsWireframe = true;
	}

	void GetVersion(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -version", Log::LogColor::Yellow);
		Console::ExecuteConsoleCommand("version");
		exit(0);
	}

	void FullScreen(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -fullscreen", Log::LogColor::Yellow);
		Application::SetFullScreen(true);
	}

	std::map<std::string, void(*)(std::vector<std::string>)> Commands =
	{
		std::pair("neverhideconsole", NeverHideConsole),
		std::pair("scene", LoadScene),
		std::pair("novsync", NoVSync),
		std::pair("wireframe", Wireframe),
		std::pair("version", GetVersion),
		std::pair("fullscreen", FullScreen),
	};
	void EvaluateLaunchArguments(std::vector<std::string> Arguments)
	{

		for (auto& i : Arguments)
		{
			Log::Print(i);
		}
		std::vector<std::string> CommandArguments;
		std::string CurrentCommand;
		for (const std::string& arg : Arguments)
		{
			if (arg[0] == '-')
			{
				if (!CurrentCommand.empty() && Commands.contains(CurrentCommand.substr(1)))
				{
					Commands[CurrentCommand.substr(1)](CommandArguments);
				}
				CommandArguments.clear();
				CurrentCommand = arg;
			}
			else if(!CurrentCommand.empty())
			{
				CommandArguments.push_back(arg);
			}
		}
		if (!CurrentCommand.empty() && Commands.contains(CurrentCommand.substr(1)))
		{
			Commands[CurrentCommand.substr(1)](CommandArguments);
		}
	}
}