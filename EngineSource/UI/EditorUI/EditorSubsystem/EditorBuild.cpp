#if EDITOR
#include "EditorBuild.h"
#include <filesystem>
#include <Engine/Build/Build.h>
#include <Engine/Utility/FileUtility.h>
#include <Engine/Utility/StringUtility.h>
#include <Engine/Subsystem/NetworkSubsystem.h>
#include <Engine/Subsystem/CSharpInterop.h>
#include <Engine/Subsystem/BackgroundTask.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/ClassesBrowser.h>
#include <UI/EditorUI/Tabs/CSharpErrorList.h>
#include <Engine/Application.h>
#include <Engine/Subsystem/Scene.h>
#include <Engine/Log.h>

bool EditorBuild::LaunchWithServer = false;
int EditorBuild::NumLaunchClients = 1;
bool EditorBuild::LaunchCurrentScene = true;
bool EditorBuild::SaveSceneOnLaunch = false;

std::string EditorBuild::BuildCMakeConfiguration(std::string Configuration, std::string Name, std::string BuildArgs)
{
	std::string MsBuildConfig = Build::CMake::GetMSBuildConfig();

#if _WIN32
	std::string ExecExtension = ".exe";
#else
	std::string ExecExtension = "";
#endif

	std::string ExecutablePath = Build::CMake::GetBuildRootPath(Configuration) + "\\";
	if (std::filesystem::exists(ExecutablePath + MsBuildConfig))
	{
		ExecutablePath += MsBuildConfig + "\\";
	}

#if !_WIN32
	StrUtil::ReplaceChar(ExecutablePath, '\\', "/");
#endif

	std::string Executable = ExecutablePath + Name + ExecExtension;

	if (!std::filesystem::exists(Executable)
		|| std::filesystem::last_write_time(Executable) < FileUtil::GetLastWriteTimeOfFolder("Code", { "x64" }))
	{
		Rebuilding = true;
		Print("Found changes to source code - building with configuration: " + Configuration, ErrorLevel::Warn);
		Build::CMake::BuildWithConfig(Configuration, BuildArgs);

		// Re-check for a configuration folder, in case this is the first time building.
		if (std::filesystem::exists(ExecutablePath + MsBuildConfig))
		{
			ExecutablePath += MsBuildConfig + "\\";
			Executable = ExecutablePath + Name + ExecExtension;
		}
	}

	return Executable;
}

EditorBuild::EditorBuild()
{
	Name = "Build";
}

void EditorBuild::Update()
{
#ifdef ENGINE_CSHARP
	if (CanHotreload == true)
	{
		Print("Finished building assembly. Reloading .dll file.", Subsystem::ErrorLevel::Info);
		CSharpInterop::CSharpSystem->ReloadCSharpAssembly();

		auto AllPanels = EditorPanel::GetAllInstancesOf<ClassesBrowser>();
		for (ClassesBrowser* c : AllPanels)
		{
			c->UpdateClasses();
			c->OnResized();
		}

		CanHotreload = false;
	}
#endif

	if (ReloadingCSharp && !CSharpReloadBox)
	{
		CSharpReloadBox = new DialogBox("C#", 0, "Rebuilding C# assembly... Check log for details.", {});
	}
	else if (!ReloadingCSharp && CSharpReloadBox)
	{
		delete CSharpReloadBox;
		CSharpReloadBox = nullptr;
	}

	if (Rebuilding && !RebuildingBox)
	{
		RebuildingBox = new DialogBox("Build", 0, "Compiling game...", {});
	}
	else if (!Rebuilding && RebuildingBox)
	{
		delete RebuildingBox;
		RebuildingBox = nullptr;
	}

}

#ifdef ENGINE_CSHARP
bool EditorBuild::RebuildAssembly()
{
	ReloadingCSharp = true;
	bool Success = EditorUI::PipeProcessToLog("cd Scripts && dotnet build", [this](std::string msg) 
		{
			HandleDotNetBuildLog(msg);
		}) == 0;
	ReloadingCSharp = false;
	CanHotreload = Success;
	return Success;
}
#endif

void EditorBuild::Run()
{
	std::string ProjectName = Build::GetProjectBuildName();
	std::string ExecutablePath = "", ServerExecutablePath = "";
	try
	{
#if !ENGINE_NO_SOURCE
		// No Solution -> no build name -> probably using CMake on windows.
		if (Build::CMake::IsUsingCMake())
		{
			ExecutablePath = BuildCMakeConfiguration("x64-Debug", ProjectName, "");
			if (NetworkSubsystem::IsActive())
			{
				ServerExecutablePath = BuildCMakeConfiguration("x64-Server", ProjectName, "-DSERVER=ON");
			}
		}
#if _WIN32
		else
		{
			ExecutablePath = "bin\\" + ProjectName + "-Debug.exe";
			if (!std::filesystem::exists(ExecutablePath)
				|| std::filesystem::last_write_time(ExecutablePath) < FileUtil::GetLastWriteTimeOfFolder("Code", { "x64" }))
			{
				Rebuilding = true;
				Print("Detected uncompiled changes to C++ code. Rebuilding...");
				if (Build::BuildCurrentSolution("Debug"))
				{
					Print("Build for configuration 'Debug' failed. Cannot launch project.", ErrorLevel::Error);
					Rebuilding = false;
					return;
				}
			}

			ServerExecutablePath = "bin\\" + ProjectName + "-Server.exe";
			if (LaunchWithServer
				&& (!std::filesystem::exists(ServerExecutablePath)
					|| std::filesystem::last_write_time(ServerExecutablePath) < FileUtil::GetLastWriteTimeOfFolder("Code", { "x64" })))
			{
				Rebuilding = true;
				if (Build::BuildCurrentSolution("Server"))
				{
					Print("Build for configuration 'Debug' failed. Cannot launch project.", ErrorLevel::Error);
					Rebuilding = false;
					return;
				}
			}
		}
#endif // WIN32
		Rebuilding = false;
#endif // !ENGINE_NO_SOURCE

#if ENGINE_NO_SOURCE || __linux__
#if !ENGINE_NO_SOURCE
		if (!Build::CMake::IsUsingCMake())
#endif
		{
			ProjectName = "Klemmgine";
			ExecutablePath = "bin\\" + ProjectName + "-Debug";
			ServerExecutablePath = "bin\\" + ProjectName + "-Server";
		}
#if _WIN32
		ExecutablePath.append(".exe");
		ServerExecutablePath.append(".exe");
#endif
#endif // ENGINE_NO_SOURCE || __linux__

#ifdef ENGINE_CSHARP
		if ((!std::filesystem::exists("CSharp/Build/CSharpAssembly.dll")
			|| std::filesystem::last_write_time("CSharp/Build/CSharpAssembly.dll") < FileUtil::GetLastWriteTimeOfFolder("Scripts", { "obj" }))
			&& CSharpInterop::GetUseCSharp())
		{
			if (!RebuildAssembly())
			{
				Print("Failed to build C# assembly.", ErrorLevel::Error);
				return;
			}
		}
#endif
	}
	catch (std::filesystem::filesystem_error& e)
	{
		Print("Filesystem error thrown when trying to check for rebuild. " + std::string(e.what()), ErrorLevel::Error);
		return;
	}

	if (SaveSceneOnLaunch)
	{
		Application::EditorInstance->ShouldSave = true;
	}

	std::string Args = LaunchInEditorArgs;
	if (LaunchCurrentScene)
	{
		Args.append(" -scene " + FileUtil::GetFileNameFromPath(Scene::CurrentScene));
	}

#if !_WIN32
	StrUtil::ReplaceChar(ExecutablePath, '\\', "/");
#endif

	// -nostartupinfo: Do not show any version information on startup, this just clutters the editor log.
	// -nocolor: Do not print any color. The editor log ignores it anyways, and on Linux this just puts color codes everywhere.
	std::string CommandLine = ExecutablePath + " -nostartupinfo -nocolor -editorPath " + Application::GetEditorPath() + " " + Args;

	if (LaunchWithServer)
	{
		CommandLine.append(" -connect localhost ");
	}

	Print("Starting process: " + CommandLine, ErrorLevel::ImportantInfo);
	int ret = 0;
	std::string Command;
#if _WIN32
	for (int i = 0; i < NumLaunchClients; i++)
	{
		Command.append("start /b " + CommandLine);
		if (i < NumLaunchClients - 1)
		{
			Command.append(" && ");
		}
	}

	// If only one new process is created, write the output of that process to the log.
	// With multiple processes this gets very confusing.
	if (NumLaunchClients == 1)
	{
		new BackgroundTask([Command]() { EditorUI::PipeProcessToLog(Command, [](std::string msg) { Log::Print("[Debug]: " + msg); }); });
	}
	else
	{
		new BackgroundTask([Command]() { int ret = system(Command.c_str()); });
	}
#else
	for (int i = 0; i < NumLaunchClients; i++)
	{
		if (NumLaunchClients == 1)
		{
			new BackgroundTask([Command]() { EditorUI::PipeProcessToLog(Command, [](std::string msg) { Log::Print("[Debug]: " + msg); }); });
		}
		else
		{
			new BackgroundTask([CommandLine]() { system(CommandLine.c_str()); });
		}
	}
#endif
	if (LaunchWithServer)
	{
#if _WIN32
		ret = system(("start " + ServerExecutablePath + " -nostartupinfo -quitondisconnect -editorPath "
			+ Application::GetEditorPath()
			+ " "
			+ Args).c_str());
#else
		ret = system(("./bin/"
			+ ProjectName
			+ "-Server -nostartupinfo -quitondisconnect -editorPath "
			+ Application::GetEditorPath()
			+ " "
			+ Args + " &").c_str());
#endif
	}
}

std::string EditorBuild::LaunchInEditorArgs;

void EditorBuild::SetSaveSceneOnLaunch(bool NewValue)
{
	SaveSceneOnLaunch = NewValue;
}

void EditorBuild::SetLaunchCurrentScene(bool NewLaunch)
{
	LaunchCurrentScene = NewLaunch;
}

void EditorBuild::HandleDotNetBuildLog(std::string Message)
{
	// /path/to/file.cs(1, 2): error CS1234: Example msbuild error message. [/path/to/proj.csproj]

	Log::Print("[C#]: [Build]: " + Message, Log::LogColor::Gray);

	size_t FirstBracket = Message.find_first_of("(");
	size_t FirstClosingBracket = Message.find_first_of(")");

	if (FirstBracket == std::string::npos || FirstClosingBracket == std::string::npos)
	{
		return;
	}

	std::string File = Message.substr(0, FirstBracket);

	if (!std::filesystem::exists(File))
	{
		return;
	}

	std::string ErrorPosition = Message.substr(FirstBracket + 1, FirstClosingBracket - FirstBracket - 1);
	std::vector<std::string> ErrorPositionStrings = StrUtil::SeparateString(ErrorPosition, ',');

	std::string MessageString = Message.substr(FirstClosingBracket + 2);

	size_t ColonPos = MessageString.find_first_of(":");

	std::vector<std::string> ErrorDescription = StrUtil::SeparateString(MessageString.substr(0, ColonPos), ' ');

	std::string Description = MessageString.substr(ColonPos + 2);
	Description = Description.substr(0, Description.find_last_of("["));

	CSharpErrorList::Message msg;

	msg.File = File;
	msg.Code = ErrorDescription[1];
	msg.Message = Description;

	auto Panels = EditorPanel::GetAllInstancesOf<CSharpErrorList>();

	for (CSharpErrorList* List : Panels)
	{
		List->AddMessage(msg);
	}
}
#endif