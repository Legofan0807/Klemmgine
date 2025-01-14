#if EDITOR
#pragma once
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/Tabs/EditorTab.h>
#include <UI/EditorUI/Toolbar.h>
#include <Engine/Log.h>
#include <Engine/EngineProperties.h>
#include <Engine/Application.h>
#include <Rendering/RenderSubsystem/CSM.h>
#include <Engine/AppWindow.h>
#include "EditorSubsystem/EditorBuild.h"

/**
* @brief
* EditorPanel responsible for loading, saving and editing project settings.
*/
class SettingsPanel : public EditorPanel
{
	UIBox* HorizontalBox = nullptr;
	struct SettingsCategory
	{
		std::string Name;

		struct Setting
		{
			std::string Name;
			NativeType::NativeType NativeType;
			std::string Value;
			void (*OnChanged)(std::string NewValue);
			size_t cat = 0, entry = 0;
		};

		std::vector<Setting> Settings;
	};

	std::vector<UIBox*> LoadedSettingElements;

	size_t SelectedSetting = 0;
	UIScrollBox* SettingsBox = nullptr;
	UIBackground* SettingsCategoryBox;
	std::vector<SettingsCategory::Setting> LoadedSettings;

	std::vector<SettingsCategory> Preferences =
	{
		SettingsCategory("Editor",
			{
			SettingsCategory::Setting("UI:Light mode [Experimental]", NativeType::Bool, "0", [](std::string NewValue)
			{
				Application::EditorInstance->SetUseLightMode(std::stoi(NewValue));
				Window::SetTitleBarDark(!std::stoi(NewValue));
			}),
			SettingsCategory::Setting("Toolbar:Show Save Button", NativeType::Bool, "1", [](std::string NewValue)
			{
				Toolbar::ToolbarInstance->SetButtonVisibility("Save", std::stoi(NewValue));
			}),
			SettingsCategory::Setting("Toolbar:Show Bake button", NativeType::Bool, "1", [](std::string NewValue)
			{
				Toolbar::ToolbarInstance->SetButtonVisibility("Bake", std::stoi(NewValue));
			}),
			SettingsCategory::Setting("Toolbar:Show Wireframe Button", NativeType::Bool, "1", [](std::string NewValue)
			{
				Toolbar::ToolbarInstance->SetButtonVisibility("Wireframe", std::stoi(NewValue));
			}),
			SettingsCategory::Setting("Toolbar:Show Build Button", NativeType::Bool, "1", [](std::string NewValue)
			{
				Toolbar::ToolbarInstance->SetButtonVisibility("Build", std::stoi(NewValue));
			}),
			SettingsCategory::Setting("Toolbar:Show run button", NativeType::Bool, "1", [](std::string NewValue)
			{
				Toolbar::ToolbarInstance->SetButtonVisibility("Run", std::stoi(NewValue));
			}),
#if ENGINE_CSHARP
			SettingsCategory::Setting("Toolbar:Show reload C# button", NativeType::Bool, "1", [](std::string NewValue)
			{
				Toolbar::ToolbarInstance->SetButtonVisibility("Reload C#", std::stoi(NewValue));
			}),
#endif
			SettingsCategory::Setting("Editor:Start in fullscreen", NativeType::Bool, "0", [](std::string NewValue)
			{
				if (Stats::Time == 0)
				{
					Window::SetFullScreen(std::stoi(NewValue));
				}
			}),
			}
		),

		SettingsCategory("CMake",
		{
#if _WIN32
			SettingsCategory::Setting("Compilation:Use MSBuild Debug configuration", NativeType::Bool, "0", [](std::string NewValue)
			{
				Build::CMake::SetMSBuildConfig(std::stoi(NewValue) ? "Debug" : "Release");
			}),
#endif
			SettingsCategory::Setting("Compilation:Processor count", NativeType::Int, "12", [](std::string NewValue)
			{
				Build::CMake::SetCompilationProcessorCount(std::stoi(NewValue));
			}),
			SettingsCategory::Setting("CMake:Executable", NativeType::String, "cmake", [](std::string NewValue)
			{
				Build::CMake::SetCMakeExecutable(NewValue);
			}),
			SettingsCategory::Setting("CMake:CMake generator (Leave empty for default)", NativeType::String, "", [](std::string NewValue)
			{
				Build::CMake::CMakeGenerator = NewValue;
			}),
			//SettingsCategory::Setting("CMake:Additional configuration arguments", NativeType::String, "", [](std::string NewValue)
			//{
			//}),
}),

		SettingsCategory("Graphics",
			{
			SettingsCategory::Setting("Graphics:Shadows", NativeType::Bool, "1", [](std::string NewValue)
			{
				Graphics::RenderShadows = std::stoi(NewValue);
				CSM::ReInit();
			}),
			SettingsCategory::Setting("Graphics:Shadow Resolution", NativeType::Int, "2000", [](std::string NewValue)
			{
				int NewVal = std::stoi(NewValue);
				if (NewVal != Graphics::ShadowResolution)
				{
					Graphics::ShadowResolution = NewVal;
					CSM::ReInit();
				}
			}),
			SettingsCategory::Setting("Graphics:Bloom", NativeType::Bool, "1", [](std::string NewValue)
			{
				Graphics::Bloom = std::stoi(NewValue);
			}),
			SettingsCategory::Setting("Graphics:Ambient Occlusion", NativeType::Bool, "1", [](std::string NewValue)
			{
				Graphics::SSAO = std::stoi(NewValue);
			}),
			SettingsCategory::Setting("Graphics:Anti aliasing", NativeType::Bool, "1", [](std::string NewValue)
			{
				Graphics::RenderAntiAlias = std::stoi(NewValue);
				Graphics::SetWindowResolution(Graphics::WindowResolution, true);
			}),
			SettingsCategory::Setting("Display:VSync", NativeType::Bool, "1", [](std::string NewValue)
			{
				Graphics::VSync = std::stoi(NewValue);
			}),
			SettingsCategory::Setting("Display:Resolution scale %", NativeType::Int, "100", [](std::string NewValue)
			{
				Graphics::ResolutionScale = (float)std::stoi(NewValue) / 100.0f;
				Graphics::SetWindowResolution(Graphics::WindowResolution);
			}),
			}
		),


		SettingsCategory("Project specific",
			{
				SettingsCategory::Setting("Run from editor:Launch arguments", NativeType::String, "", [](std::string NewValue)
				{
					EditorBuild::LaunchInEditorArgs = NewValue;
				}),
				SettingsCategory::Setting("Run from editor:Launch scene that is currently loaded in editor", NativeType::Bool, "1", [](std::string NewValue)
				{
					EditorBuild::SetLaunchCurrentScene(NewValue == "1");
				}),
				SettingsCategory::Setting("Run from editor:Save current scene before launch", NativeType::Bool, "0", [](std::string NewValue)
				{
					EditorBuild::SetSaveSceneOnLaunch(NewValue == "1");
				}),
				SettingsCategory::Setting("Networking:Start server on launch", NativeType::Bool, "0", [](std::string NewValue)
				{
					EditorBuild::LaunchWithServer = std::stoi(NewValue);
				}),
				SettingsCategory::Setting("Networking:Number of clients", NativeType::Int, "1", [](std::string NewValue)
				{
					EditorBuild::NumLaunchClients = std::stoi(NewValue);
				}),

#if ENGINE_CSHARP && !ENGINE_NO_SOURCE
				SettingsCategory::Setting("C#:Use C# in project (Requires restart)", NativeType::Bool, "1", [](std::string NewValue)
				{
				}),
#endif
			}
		),

	};

	void GenerateUI();
	void GenerateSection(UIBox* Parent, std::string Name, int Index, NativeType::NativeType SectionType, std::string Value);
public:
	static void NewSettingsPanel();
	void OpenSettingsPage(std::string Name);
	void Load();
	void Save();
	void OnResized() override;

	void OnButtonClicked(int Index) override;
	SettingsPanel(EditorPanel* Parent);
};
#endif