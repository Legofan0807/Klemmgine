#ifdef EDITOR
#include "EditorUI.h"
#include "Engine/Utility/FileUtility.h"
#include <filesystem>
#include "Engine/Subsystem/Scene.h"
#include <Engine/EngineProperties.h>
#include <UI/UIScrollBox.h>	
#include <UI/EditorUI/LogUI.h>
#include <UI/EditorUI/Toolbar.h>
#include <UI/EditorUI/AssetBrowser.h>
#include <UI/EditorUI/ClassesBrowser.h>
#include <UI/EditorUI/StatusBar.h>
#include <UI/EditorUI/Viewport.h>
#include <UI/EditorUI/ObjectList.h>
#include <UI/EditorUI/ContextMenu.h>
#include <atomic>
#include <thread>
#include <Engine/Input.h>
#include <UI/EditorUI/Popups/DialogBox.h>
#include <Engine/Subsystem/Console.h>
#include <Rendering/RenderSubsystem/BakedLighting.h>
#include <Engine/File/Assets.h>
#include <SDL.h>
#include <Rendering/Texture/Texture.h>
#include <Engine/Application.h>
#include <UI/EditorUI/SettingsPanel.h>
#include <UI/EditorUI/SerializePanel.h>
#include <Engine/Utility/StringUtility.h>

TextRenderer* EditorUI::Text = nullptr;
TextRenderer* EditorUI::MonoText = nullptr;
EditorPanel* EditorUI::RootPanel = nullptr;
std::vector<unsigned int> EditorUI::Textures;
std::vector<SceneObject*> EditorUI::SelectedObjects;
EditorUI::CursorType EditorUI::CurrentCursor = CursorType::Default;

// TODO: Move run-in-editor and C# hotreload stuff into subsystems. (for prettier logging and cleaner code)

Vector3 EditorUI::UIColors[NumUIColors] =
{
	Vector3(0.125f, 0.125f, 0.13f),	// Default background
	Vector3(0.08f),					// Dark background
	Vector3(1),						// Highlight color
	Vector3(0.2f),					// Brighter background
};

namespace Editor
{
	Vector2 DragMinMax;
	Vector2 NewDragMinMax = DragMinMax;
	bool IsSavingScene = false;

	Vector3 NewUIColors[EditorUI::NumUIColors] =
	{
		Vector3(0.85f, 0.85f, 0.85f),	//Default background
		Vector3(0.6f),				//Dark background
		Vector3(0),					//Highlight color,
		Vector3(1)
	};

	bool LightMode = false;

	static Vector3 ReplaceWithNewUIColor(Vector3 PrevColor)
	{
		for (uint8_t i = 0; i < EditorUI::NumUIColors; i++)
		{
			if (Application::EditorInstance->UIColors[i] == PrevColor)
			{
				return Editor::NewUIColors[i];
			}
		}

		return PrevColor;
	}

	struct ProcessInfo
	{
		std::string Command;
		FILE* Pipe = nullptr;
		std::thread* Thread = nullptr;
		std::atomic<bool> Active = false;
		std::function<void(std::string)> PrintFunction;
		bool Async = false;
	};

	static int ReadProcessPipe(FILE* p, ProcessInfo* Info)
	{
		std::string CurrentMessage;
		while (!feof(p))
		{
			char NewChar = (char)fgetc(p);

			if (NewChar == '\n')
			{
				Info->PrintFunction(CurrentMessage);
				CurrentMessage.clear();
			}
			else
			{
				CurrentMessage.append({ NewChar });
			}
		}
#if _WIN32
		int ret = _pclose(p);
#else
		int ret = pclose(p);
#endif
		Info->Active = false;

		return ret;
	}
}

namespace UI
{
	extern std::vector<UIBox*> UIElements;
}

bool EditorUI::ChangedScene = false;
bool EditorUI::IsBakingScene = false;

void EditorUI::LaunchInEditor()
{
	static_cast<EditorBuild*>(Subsystem::GetSubsystemByName("Build"))->Run();
}

void EditorUI::SaveCurrentScene()
{
	if (Scene::CurrentScene.empty())
	{
		Application::EditorInstance->Print("Saving scene \"Untitled\"", ErrorLevel::ImportantInfo);
		Scene::SaveSceneAs("Content/Untitled");
		Scene::CurrentScene = "Content/Untitled";
	}
	else
	{
		Application::EditorInstance->Print("Saving scene \"" + FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene) + "\"", ErrorLevel::ImportantInfo);
		Scene::SaveSceneAs(Scene::CurrentScene);
	}
	ChangedScene = false;
	AssetBrowser::UpdateAll();
	EditorPanel::UpdateAllInstancesOf<ObjectList>();
}


void EditorUI::OpenScene(std::string NewScene)
{
	if (ChangedScene)
	{
		new DialogBox(FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene),
			0, 
			"The current scene has unsaved changes. Save these changes before loading a new scene?",
			{
				EditorPopup::PopupOption("Yes", [NewScene]()
				{
					SaveCurrentScene(); 
					ChangedScene = false;
					Scene::LoadNewScene(NewScene, true);
					Viewport::ViewportInstance->ClearSelectedObjects();
					EditorPanel::UpdateAllInstancesOf<ContextMenu>();
				}),

				EditorPopup::PopupOption("No", [NewScene]()
				{
					ChangedScene = false;
					Scene::LoadNewScene(NewScene, true);
					Viewport::ViewportInstance->ClearSelectedObjects();
					EditorPanel::UpdateAllInstancesOf<ContextMenu>();
				}),

				EditorPopup::PopupOption("Cancel", nullptr)
			});
		return;
	}

	ChangedScene = false;
	Scene::LoadNewScene(NewScene, true);

	EditorUI::SelectedObjects.clear();

	EditorPanel::UpdateAllInstancesOf<ContextMenu>();
}

bool EditorUI::GetUseLightMode()
{
	return Editor::LightMode;
}

void EditorUI::SetUseLightMode(bool NewLightMode)
{
	if (NewLightMode != Editor::LightMode)
	{
		for (auto i : UI::UIElements)
		{
			if (dynamic_cast<UIText*>(i))
			{
				static_cast<UIText*>(i)->SetColor(Editor::ReplaceWithNewUIColor(static_cast<UIText*>(i)->GetColor()));
			}
			if (dynamic_cast<UIButton*>(i))
			{
				static_cast<UIButton*>(i)->SetColor(Editor::ReplaceWithNewUIColor(static_cast<UIButton*>(i)->GetColor()));
			}
			if (dynamic_cast<UIBackground*>(i))
			{
				static_cast<UIBackground*>(i)->SetColor(Editor::ReplaceWithNewUIColor(static_cast<UIBackground*>(i)->GetColor()));
			}
			if (dynamic_cast<UITextField*>(i))
			{
				static_cast<UITextField*>(i)->SetColor(Editor::ReplaceWithNewUIColor(static_cast<UITextField*>(i)->GetTextColor()));
				static_cast<UITextField*>(i)->SetTextColor(Editor::ReplaceWithNewUIColor(static_cast<UITextField*>(i)->GetTextColor()));
			}
		}
		std::swap(Application::EditorInstance->UIColors, Editor::NewUIColors);
		Editor::LightMode = NewLightMode;
		Application::EditorInstance->Print("Toggled light mode", Subsystem::ErrorLevel::Warn);
	}
	UIBox::ForceUpdateUI();
}

void EditorUI::LoadDefaultLayout()
{
	if (RootPanel)
	{
		delete RootPanel;
	}

	RootPanel = new EditorPanel(-1, Vector2(2, 1.95f), "root", "root");
	RootPanel->ChildrenAlign = EditorPanel::ChildrenType::Horizontal;

	EditorPanel* RightPanel = new EditorPanel(RootPanel, "panel");
	RightPanel->Size = 0.3f;

	(new AssetBrowser(RightPanel));
	(new ClassesBrowser(RightPanel));

	EditorPanel* CenterPanel = new EditorPanel(RootPanel, "panel");
	CenterPanel->ChildrenAlign = EditorPanel::ChildrenType::Vertical;
	CenterPanel->Size = 1.425f;
	(new LogUI(CenterPanel))->Size = 0.4f;
	(new Viewport(CenterPanel))->Size = 1.425f;
	new Toolbar(CenterPanel);
	EditorPanel* LeftPanel = new EditorPanel(RootPanel, "panel");
	LeftPanel->ChildrenAlign = EditorPanel::ChildrenType::Vertical;
	EditorPanel* BottomLeftPanel = new EditorPanel(LeftPanel, "panel");
	new ContextMenu(BottomLeftPanel, false);
	new ContextMenu(BottomLeftPanel, true);
	BottomLeftPanel->Size = 0.8f;
	new ObjectList(LeftPanel);

	RootPanel->OnPanelResized();
}

void EditorUI::LoadPanelLayout(EditorPanel* From)
{
	if (RootPanel)
	{
		delete RootPanel;
		RootPanel = nullptr;
	}
	RootPanel = From;
}

int EditorUI::PipeProcessToLog(std::string Command, std::function<void(std::string)> PrintFunction)
{
	using namespace Editor;

#if _WIN32
	FILE* process = _popen(Command.c_str(), "r");
#else
	auto process = popen(Command.c_str(), "r");
#endif

	ProcessInfo proc;

	proc.Active = true;
	proc.Command = Command;
	proc.Pipe = process;
	proc.PrintFunction = PrintFunction;
	return ReadProcessPipe(process, &proc);
}

void EditorUI::CreateFile(std::string Path, std::string Name, std::string Ext)
{
	std::string Addition;
	size_t AdditionNum = 0;
	while (std::filesystem::exists(Path + "/" + Name + Addition + "." + Ext))
	{
		Addition = "_" + std::to_string(++AdditionNum);
	}

	std::ofstream out = std::ofstream(Path + "/" + Name + Addition + "." + Ext);
	out.close();
}

EditorUI::EditorUI()
{
	Application::EditorInstance = this;
	Name = "UISystem";
	Text = new TextRenderer(Application::GetEditorPath() + "/EditorContent/EditorFont.ttf");
	MonoText = new TextRenderer(Application::GetEditorPath() + "/EditorContent/EditorMonospace.ttf");
	Input::CursorVisible = true;
	LoadEditorTextures();

	Subsystem::Load(new EditorBuild());

	if (std::filesystem::exists(Editor::SerializePanel::GetLayoutPrefFilePath() + ".pref"))
	{
		SaveData s = SaveData(Editor::SerializePanel::GetLayoutPrefFilePath(), "pref", false, false);
		EditorUI::LoadPanelLayout(Editor::SerializePanel::DeSerializeLayout(s.GetField("root")));
	}
	else
	{
		LoadDefaultLayout();
	}
	new StatusBar();
	delete new SettingsPanel(nullptr);

	Cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	Cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	Cursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	Cursors[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
	Cursors[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	Cursors[5] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	Cursors[6] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	Cursors[7] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

	Console::ConsoleSystem->RegisterCommand(Console::Command("build", []() {new std::thread(Build::TryBuildProject, "GameBuild/"); }, {}));
	Console::ConsoleSystem->RegisterCommand(Console::Command("save", []() {
		std::string Old = Scene::CurrentScene;
		if (!Console::ConsoleSystem->CommandArgs().empty())
		{
			Scene::CurrentScene = "Content/" + Console::ConsoleSystem->CommandArgs()[0];
		}

		EditorUI::SaveCurrentScene();
		Scene::CurrentScene = Old;
		}, { Console::Command::Argument("scene_file_path", NativeType::String, true)}));
	Console::ConsoleSystem->RegisterCommand(Console::Command("dump_editor_layout", [this]() 
		{
			Print(Editor::SerializePanel::SerializeLayout(RootPanel).Serialize(0));
		}, {}));

	Console::ConsoleSystem->RegisterCommand(Console::Command("toggle_light", []() { EditorUI::SetUseLightMode(!EditorUI::GetUseLightMode()); }, {}));
#ifdef ENGINE_CSHARP
	Console::ConsoleSystem->RegisterCommand(Console::Command("run", EditorUI::LaunchInEditor, {}));

#endif
}

EditorUI::~EditorUI()
{
}

void EditorUI::OnLeave()
{
	if (ChangedScene)
	{
		new DialogBox("Scene", 0,
			"The current scene has unsaved changes. Save changes before exiting?",
			{
				EditorPopup::PopupOption("Yes", []() { SaveCurrentScene(); Application::Quit(); }),
				EditorPopup::PopupOption("No", &Application::Quit),
				EditorPopup::PopupOption("Cancel", nullptr)
			});
	}
	else
	{
		Application::Quit();
	}
}

void EditorUI::Update()
{
	EditorPanel::TickPanels();

	if (Scene::CurrentScene.empty())
	{
		Window::SetWindowTitle(StrUtil::Format("No scene - %s", Scene::CurrentScene.c_str(), Project::ProjectName));
	}
	else
	{
		Window::SetWindowTitle(StrUtil::Format("%s.jscn - %s", Scene::CurrentScene.c_str(), Project::ProjectName));
	}

	if (BakedLighting::FinishedBaking)
	{
		BakedLighting::FinishedBaking = false;
		EditorUI::IsBakingScene = false;
		Assets::ScanForAssets();
		BakedLighting::LoadBakeFile(FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene));
	}

	if (dynamic_cast<UIButton*>(UI::HoveredBox))
	{
		CurrentCursor = CursorType::Grab;
	}
	if (dynamic_cast<UITextField*>(UI::HoveredBox))
	{
		CurrentCursor = CursorType::TextHover;
	}
	if (UIScrollBox::IsDraggingScrollBox)
	{
		CurrentCursor = CursorType::Default;
	}
	if (CurrentCursor < CursorType::End && (int)CurrentCursor >= 0)
	{
		SDL_SetCursor(Cursors[(int)CurrentCursor]);
	}
	CurrentCursor = CursorType::Default;

	if (DraggedItem)
	{
		DraggedItem->SetPosition(Input::MouseLocation - DraggedItem->GetUsedSize() / 2);
		if (!Input::IsLMBDown)
		{
			delete DraggedItem;
			DraggedItem = nullptr;
		}
	}

	if ((Input::IsKeyDown(Input::Key::LCTRL) && Input::IsKeyDown(Input::Key::s) && !Input::IsRMBDown) || ShouldSave)
	{
		if (ChangedScene)
		{
			SaveCurrentScene();
			ShouldSave = false;
		}
		Editor::IsSavingScene = true;
	}
	else
	{
		Editor::IsSavingScene = false;
	}
}

void EditorUI::OnObjectSelected()
{
	EditorPanel::UpdateAllInstancesOf<ContextMenu>();
	EditorPanel::UpdateAllInstancesOf<ObjectList>();
}

std::string EditorUI::ToShortString(float val)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << val;
	return stream.str();
}


void EditorUI::LoadEditorTextures()
{
	const int ImageSize = 28;
	std::string Images[ImageSize]
	{								//Texture Indices
		"CPPClass.png",				//00 -> C++ class icon
		"Wireframe.png",			//01 -> Symbol for button to toggle wireframe
		"Save.png",					//02 -> Save Button
		"Build.png",				//03 -> Package button
		"X.png",					//04 -> X Symbol
		"Folder.png",				//05 -> Folder symbol for asset browser
		"Sound.png",				//06 -> Sound symbol for asset browser
		"Scene.png",				//07 -> Scene symbol for asset browser
		"ExitFolder.png",			//08 -> Icon used to navigate back one folder
		"Material.png",				//09 -> Material symbol for asset browser
		"MaterialTemplate.png",		//10 -> Material Template symbol for asset browser
		"Model.png",				//11 -> Model symbol for asset browser
		"Reload.png",				//12 -> Reload symbol
		"ExpandedArrow.png",		//13 -> Expanded arrow
		"CollapsedArrow.png",		//14 -> Collapsed arrow
		"Placeholder.png",			//15 -> Placeholder
		"Checkbox.png",				//16 -> Checked checkbox
		"Cubemap.png",				//17 -> CubeMap icon
		"Texture.png",				//18 -> Texture icon
		"Particle.png",				//19 -> Particle icon
		"Settings.png",				//20 -> Settings icon
		"Play.png",					//21 -> Play icon
		"CSharpClass.png",			//22 -> CSharp class icon
		"WindowX.png",				//23 -> Window X icon
		"WindowResize.png",			//24 -> Window Resize icon
		"WindowResize2.png",		//25 -> Window FullScreen Resize icon
		"WindowMin.png",			//26 -> Window Minimize icon
		"Lighting.png"				//27 -> Lightmap icon	
	};

	for (int i = 0; i < Textures.size(); i++)
	{
		Texture::UnloadTexture(Textures[i]);
	}
	for (int i = 0; i < ImageSize; i++)
	{
		Textures.push_back(Texture::LoadTexture(Application::GetEditorPath() + "/EditorContent/Images/" + Images[i]));
	}
}

void EditorUI::BakeScene()
{
	EditorUI::IsBakingScene = true;
	BakedLighting::BakeCurrentSceneToFile();
}

void EditorUI::OnResized()
{
	RootPanel->OnPanelResized();
}

#endif