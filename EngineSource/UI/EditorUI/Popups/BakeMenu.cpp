#if EDITOR
#include "BakeMenu.h"
#include <UI/EditorUI/EditorUI.h>
#include <Rendering/Utility/BakedLighting.h>
#include <Engine/FileUtility.h>
#include <filesystem>
#include <UI/UIScrollBox.h>
#include <Engine/Input.h>
#include <Engine/Log.h>


bool BakeMenu::BakeMenuActive = false;

BakeMenu::BakeMenu()
	: EditorPanel(Editor::CurrentUI->UIColors, Position, Vector2(0.5, 0.55), Vector2(0.5, 0.55), 2, true, "Bake Lightmap")
{
	if (BakeMenuActive)
	{
		return;
	}

	BakeMenuActive = true;
	ButtonBackground = new UIBackground(true, 0, UIColors[0] * 1.5);
	ButtonBackground->SetPadding(0);
	ButtonBackground->SetBorder(UIBox::E_DARKENED_EDGE, 0.2);
	TabBackground->Align = UIBox::E_DEFAULT;
	TabBackground->AddChild(ButtonBackground);

	ButtonBackground->AddChild((new UIButton(true, 0, UIColors[2], this, -2))
		->SetPadding(0.01)
		->SetBorder(UIBox::E_ROUNDED, 0.2)
		->AddChild((new UIText(0.45, 1 - UIColors[2], "Bake", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.005)))
		->AddChild((new UIButton(true, 0, UIColors[2], this, -1))
			->SetPadding(0.01)
			->SetBorder(UIBox::E_ROUNDED, 0.2)
			->AddChild((new UIText(0.45, 1 - UIColors[2], "Cancel", Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005)));

	InputFields[0] = new UITextField(true, 0, UIColors[1], this, 0, Editor::CurrentUI->EngineUIText);
	InputFields[1] = new UITextField(true, 0, UIColors[1], this, 0, Editor::CurrentUI->EngineUIText);



	TabBackground->AddChild((new UIBox(true, 0))
		->SetPadding(0.01, 0.3, 0.01, 0.01)
		->AddChild((new UIText(0.55, UIColors[2], "Lightmap scale:      ", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.005))
		->AddChild(InputFields[0]
			->SetText(EditorUI::ToShortString(BakedLighting::LightmapScaleMultiplier))
			->SetTextSize(0.5)
			->SetPadding(0, 0, 0, 0)
			->SetMinSize(Vector2(0.1, 0.01))));

	TabBackground->AddChild((new UIBox(true, 0))
		->SetPadding(0.01)
		->AddChild((new UIText(0.55, UIColors[2], "Lightmap resolution: ", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.005))
		->AddChild(InputFields[1]
			->SetText(std::to_string(BakedLighting::LightmapResolution))
			->SetTextSize(0.5)
			->SetPadding(0, 0, 0, 0)
			->SetMinSize(Vector2(0.1, 0.01))));

	UpdateLayout();
}

void BakeMenu::UpdateLayout()
{
	if (ButtonBackground)
	{
		ButtonBackground->SetMinSize(Vector2(TabBackground->GetMinSize().X, 0.075));
	}
}

BakeMenu::~BakeMenu()
{
	BakeMenuActive = false;
}

void BakeMenu::OnButtonClicked(int Index)
{
	if (Index == -1)
	{
		delete this;
		return;
	}
	if (Index == -2)
	{
		BakedLighting::LightmapScaleMultiplier = std::stof(InputFields[0]->GetText());
		BakedLighting::LightmapResolution = std::stoull(InputFields[1]->GetText());
		Editor::CurrentUI->BakeScene();
		StartBake();
		return;
	}
}

void BakeMenu::Tick()
{
	UpdatePanel();

	if (BakeProgressText)
	{
		BakeProgressText->SetText("Progress: " + std::to_string((int)(BakedLighting::GetBakeProgress() * 100)) + "%");

		if (!IsFinished && !Editor::IsBakingScene)
		{
			IsFinished = true;
			TabBackground->AddChild((new UIButton(true, 0, UIColors[2], this, -1))
				->SetPadding(0.01)
				->SetBorder(UIBox::E_ROUNDED, 0.2)
				->AddChild((new UIText(0.45, 1 - UIColors[2], "Close", Editor::CurrentUI->EngineUIText))
					->SetPadding(0.005)));
		}
	}

	if (BakedLighting::GetBakeLog().size() != CurrentBakeProgress && LogScrollBox)
	{
		CurrentBakeProgress = BakedLighting::GetBakeLog().size();
		GenerateBakeLog();
	}
}

void BakeMenu::GenerateBakeLog()
{
	LogScrollBox->DeleteChildren();
	for (auto& i : BakedLighting::GetBakeLog())
	{
		LogScrollBox->AddChild((new UIText(0.45, UIColors[2], i, Editor::CurrentUI->EngineUIText))
			->SetPadding(0, 0, 0.01, 0));
	}
}

void BakeMenu::StartBake()
{
	TabBackground->DeleteChildren();
	ButtonBackground = nullptr;
	
	TabBackground->Align = UIBox::E_REVERSE;

	TabBackground->AddChild((new UIText(0.6, UIColors[2], "Baking lighting...", Editor::CurrentUI->EngineUIText))
		->SetPadding(0.01, 0.01, 0.02, 0.005));

	BakeProgressText = new UIText(0.5, UIColors[2], "Progress: 0%", Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(BakeProgressText
		->SetPadding(0.01, 0.01, 0.02, 0.005));

	LogScrollBox = new UIScrollBox(false, 0, 25);
	LogScrollBox->SetMinSize(Vector2(0.45, 0.35));
	LogScrollBox->SetMaxSize(Vector2(0.45, 0.35));
	LogScrollBox->Align = UIBox::E_REVERSE;
	LogScrollBox->SetPadding(0);

	TabBackground->AddChild((new UIBackground(true, 0, UIColors[1], 0))
		->SetPadding(0.01, 0.01, 0.02, 0.01)
		->AddChild(LogScrollBox));
}
#endif