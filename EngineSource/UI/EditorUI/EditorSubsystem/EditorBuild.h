#pragma once
#include "EditorSubsystem.h"
#include <UI/EditorUI/Popups/DialogBox.h>
#include <atomic>

class EditorBuild : public EditorSubsystem
{
public:
	EditorBuild();

	void Update() override;

	void Run();
	static bool LaunchWithServer;

#ifdef ENGINE_CSHARP
	bool RebuildAssembly();
#endif

	static void SetSaveSceneOnLaunch(bool NewValue);
	static std::string LaunchInEditorArgs;
	static void SetLaunchCurrentScene(bool NewLaunch);
	static int NumLaunchClients;

private:

#ifdef ENGINE_CSHARP
	void HandleDotNetBuildLog(std::string Message);
#endif

	static bool LaunchCurrentScene;
	static bool SaveSceneOnLaunch;

	std::atomic<bool> CanHotreload = false;

	std::string BuildCMakeConfiguration(std::string Configuration, std::string Name, std::string BuildArgs);

	bool ReloadingCSharp = false;
	DialogBox* CSharpReloadBox = nullptr;

	bool Rebuilding = false;
	DialogBox* RebuildingBox = nullptr;

};