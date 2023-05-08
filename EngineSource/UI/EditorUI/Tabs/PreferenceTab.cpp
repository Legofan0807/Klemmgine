#if EDITOR
#include "PreferenceTab.h"
#include <Engine/Save.h>
#include <UI/EditorUI/UIVectorField.h>
#include <Engine/Log.h>

void PreferenceTab::GenerateUI()
{
	float SegmentSize = TabBackground->GetMinSize().X - 0.5;

	TabBackground->DeleteChildren();
	auto SettingsCategoryBox = new UIBackground(false, 0, UIColors[0] * 1.2, Vector2(0, TabBackground->GetUsedSize().Y - 0.2));
	SettingsCategoryBox->Align = UIBox::E_REVERSE;
	TabBackground->AddChild(SettingsCategoryBox);

	for (size_t i = 0; i < Preferences.size(); i++)
	{
		SettingsCategoryBox->AddChild(
			(new UIButton(true, 0, UIColors[1] + (float)(i == SelectedSetting) / 8.f, this, i))->
			SetPadding(0.005, 0.005, 0.01, 0.01)->SetBorder(UIBox::E_ROUNDED, 0.5)->
			AddChild((new UIText(0.5, 1, Preferences[i].Name, Renderer))->SetTextWidthOverride(0.2)->SetPadding(0.01)));
	}

	auto SettingsBox = new UIBox(false, 0);
	SettingsBox->Align = UIBox::E_REVERSE;
	SettingsBox->SetMinSize(Vector2(0, TabBackground->GetUsedSize().Y - 0.2));
	TabBackground->AddChild(SettingsBox);

	SettingsBox->AddChild((new UIText(1, UIColors[2], "Settings/" + Preferences[SelectedSetting].Name, Renderer))
		->SetPadding(0, 0, 0, 0));

	SettingsBox->AddChild((new UIBackground(true, 0, UIColors[2], Vector2(SegmentSize, 0.005)))->SetPadding(0, 0.1, 0, 0));

	std::map<std::string, std::vector<SettingsCategory::Setting>> Categories;

	// Copy the array so it can be modifed.
	auto SettingsArray = Preferences[SelectedSetting].Settings;
	for (auto& i : SettingsArray)
	{
		auto Colon = i.Name.find_last_of(":");
		std::string CategoryName = "No category";
		if (Colon != std::string::npos)
		{
			CategoryName = i.Name.substr(0, Colon);
			i.Name = i.Name.substr(Colon + 1);
		}
		if (!Categories.contains(CategoryName))
		{
			Categories.insert(std::pair(CategoryName, std::vector<SettingsCategory::Setting>({ i })));
		}
		else
		{
			Categories[CategoryName].push_back(i);
		}
	}

	size_t CurentCategory = 0;
	for (auto& cat : Categories)
	{
		SettingsBox->AddChild(
			(new UIButton(true, 0, UIColors[1], this, -400 + CurentCategory))
				->SetPadding(0.01, 0.01, 0, 0)
				->SetMinSize(Vector2(SegmentSize, 0))
				->AddChild((new UIText(0.7, 1, "> " + cat.first, Renderer))
					->SetPadding(0.01)));

		for (size_t i = 0; i < cat.second.size(); i++)
		{
			try
			{
				GenerateSection(SettingsBox, cat.second[i].Name, -200 + i, cat.second[i].Type, cat.second[i].Value);
			}
			catch (std::exception& e)
			{
				cat.second[i].Value = "0";
			}
		}
		CurentCategory++;
	}
}

void PreferenceTab::GenerateSection(UIBox* Parent, std::string Name, int Index, Type::TypeEnum SectionType, std::string Value)
{
	Parent->AddChild((new UIText(0.7, UIColors[2], Name, Renderer))->SetPadding(0.05, 0.02, 0.05, 0.02));
	switch (SectionType)
	{
	case Type::E_FLOAT:
		break;
	case Type::E_INT:
		break;
	case Type::E_STRING:
		break;
	case Type::E_VECTOR3:
	case Type::E_VECTOR3_COLOR:
		Parent->AddChild((new UIVectorField(0, Vector3::stov(Value), this, Index, Renderer))
			->SetValueType(SectionType == Type::E_VECTOR3 ? UIVectorField::E_XYZ : UIVectorField::E_RGB)
			->SetPadding(0.02, 0.02, 0.05, 0.02));
		break;
	case Type::E_BOOL:
		Parent->AddChild((new UIButton(true, 0, 1, this, Index))
			->SetUseTexture(std::stoi(Value), Editor::CurrentUI->Textures[16])
			->SetSizeMode(UIBox::E_PIXEL_RELATIVE)
			->SetMinSize(0.05)
			->SetPadding(0.02, 0.02, 0.05, 0.02)
			->SetBorder(UIBox::E_ROUNDED, 0.5));
		break;
	default:
		break;
	}
}

void PreferenceTab::OpenSettingsPage(std::string Name)
{
	for (size_t i = 0; i < Preferences.size(); i++)
	{
		if (Preferences[i].Name == Name)
		{
			SelectedSetting = i;
			GenerateUI();
			return;
		}
	}
}

void PreferenceTab::UpdateLayout()
{
	GenerateUI();
}

void PreferenceTab::OnButtonClicked(int Index)
{
	if (Index >= 0)
	{
		SelectedSetting = Index;
		GenerateUI();
	}
	else if (Index >= -200)
	{
		switch (Preferences[0].Settings[Index + 200].Type)
		{
		case Type::E_BOOL:
			Preferences[0].Settings[Index + 200].Value = std::to_string(!((bool)std::stoi(Preferences[0].Settings[Index + 200].Value)));
		default:
			break;
		}
		Preferences[0].Settings[Index + 200].OnChanged(Preferences[0].Settings[Index + 200].Value);
	}
}

PreferenceTab::PreferenceTab(Vector3* UIColors, TextRenderer* Renderer) : EditorTab(UIColors)
{
	this->Renderer = Renderer;
	TabBackground->SetHorizontal(true);
	GenerateUI();
}

void PreferenceTab::Load(std::string File)
{
	SaveGame Pref = SaveGame("EditorContent/Config/EditorPrefs", "pref", false);
	for (auto& cat : Preferences)
	{
		for (auto& i : cat.Settings)
		{
			std::string NameCopy = i.Name;
			while (true)
			{
				auto Space = NameCopy.find_first_of(" ");
				if (Space == std::string::npos)
				{
					break;
				}
				NameCopy[Space] = '_';
			}
			Log::Print(NameCopy);
			if (Pref.GetPropterty(NameCopy).Type != Type::E_NULL)
			{
				Log::Print(NameCopy);
				i.Value = Pref.GetPropterty(NameCopy).Value;
				i.OnChanged(i.Value);
			}
		}
	}
}

void PreferenceTab::Save()
{
	SaveGame Pref = SaveGame("EditorContent/Config/EditorPrefs", "pref", false);
	for (auto& cat : Preferences)
	{
		for (auto i : cat.Settings)
		{
			while (true)
			{
				auto Space = i.Name.find_first_of(" ");
				if (Space == std::string::npos)
				{
					break;
				}
				i.Name[Space] = '_';
			}
			Pref.SetPropterty(SaveGame::SaveProperty(i.Name, i.Value, i.Type));
		}
	}
}
#endif