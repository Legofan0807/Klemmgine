#if EDITOR
#include "ParticleEditorTab.h"
#include <Engine/Stats.h>
#include <UI/UIButton.h>
#include <UI/EditorUI/UIVectorField.h>
#include <UI/UITextField.h>
#include <Engine/Log.h>
#include <Engine/File/Assets.h>
#include <filesystem>
#include <Rendering/Mesh/Model.h>
#include <UI/UIText.h>
#include <Engine/Utility/FileUtility.h>
#include <UI/UIScrollBox.h>
#include <Rendering/Mesh/Mesh.h>

/*
* TODO: Rewrite - This code is bad.
* The current UI is bad and very ugly.
*/
void ParticleEditorTab::UpdateLayout()
{
	ParticleViewport->SetPosition(TabBackground->GetPosition());
	Generate();
}

ParticleEditorTab::ParticleEditorTab(Vector3* UIColors, TextRenderer* Text, unsigned int RemoveTexture, unsigned int ReloadTexture) : EditorTab(UIColors)
{
	ParticleFramebufferObject = new FramebufferObject();
	ParticleFramebufferObject->UseMainWindowResolution = true;
	Camera* Cam = new Camera(2, 1600, 900);
	Cam->Position = glm::vec3(15, 0, 40);
		Cam->yaw = 90;
	ParticleFramebufferObject->FramebufferCamera = Cam;
	
	this->RemoveTexture = RemoveTexture;
	Particle = new Particles::ParticleEmitter();
	ParticleViewport = new UIBackground(true, Vector2(-0.7f, -0.6f), 0.9999f, Vector2(0.4f));
	ParticleFramebufferObject->ParticleEmitters.push_back(Particle);
	ParticleFramebufferObject->ReInit();
	ParticleViewport->IsVisible = false;
	ParticleViewportText = new UIText(0.4f, Vector3(0, 0.5f, 1), "Particle", Text);
	ParticleViewportText->SetPadding(0.01f);
	SelectedElementText = new UIText(0.8f, UIColors[2], "Particle has no elements", Text);
	SelectedElementText->IsVisible = false;
	SelectedElementText->SetPosition(Vector2(TabBackground->GetPosition() + TabBackground->GetUsedSize() * Vector2(0.5f, 0.9f)));
	ParticleViewport->AddChild(ParticleViewportText);
	TabText = Text;
	Generate();
}

void ParticleEditorTab::Tick()
{
	ParticleFramebufferObject->FramebufferCamera = Graphics::MainCamera;
	ParticleFramebufferObject->Active = TabBackground->IsVisible;
	ParticleViewport->SetUseTexture(true, ParticleFramebufferObject->GetTextureID());
	ParticleViewport->IsVisible = TabBackground->IsVisible;

	for (auto* elem : SettingsButtons)
	{
		elem->IsVisible = TabBackground->IsVisible;
	}
	for (auto* elem : GeneratedUI)
	{
		elem->IsVisible = TabBackground->IsVisible;
	}
	ParticleSettingsScrollBox->IsVisible = TabBackground->IsVisible;
	SelectedElementText->IsVisible = TabBackground->IsVisible;
	if (TabBackground->IsVisible)
	{
		UIBox::RedrawUI();
	}
	ParticleViewportText->SetText("Particle: " + FileUtil::GetFileNameWithoutExtensionFromPath(CurrentSystemFile)
		+ " (" + std::to_string(Particle->ParticleElements.size()) + "/255 elements)");

	if (!Particle->IsActive)
	{
		ReactivateDelay -= Performance::DeltaTime;
		if (ReactivateDelay < 0)
		{
			Particle->Reset();
			ReactivateDelay = 1.f;
		}
	}
}

void ParticleEditorTab::Load(std::string File)
{
	CurrentSystemFile = File;
	Graphics::MainCamera->Position = Vector3(0, 4, 15);
	Graphics::MainCamera->SetRotation(Vector3(0, -90, 0));
	for (unsigned int i = 0; i < Particle->ParticleVertexBuffers.size(); i++)
	{
		delete Particle->ParticleVertexBuffers[i];
	}
	Particle->ParticleVertexBuffers.clear();
	Particle->SpawnDelays.clear();
	Particle->ParticleInstances.clear();
	Particle->Contexts.clear();
	Particle->ParticleMatrices.clear();
	Particle->ParticleElements.clear();

	if (std::filesystem::exists(File))
	{
		if (!std::filesystem::is_empty(File))
		{
			auto ParticleData = Particles::ParticleEmitter::LoadParticleFile(File, ElementMaterials);
			for (unsigned int i = 0; i < ParticleData.size(); i++)
			{
				Particle->AddElement(ParticleData[i], Material::LoadMaterialFile(ElementMaterials[i]));
			}
		}
	}
	Generate();
}

void ParticleEditorTab::ReloadMesh()
{
}

void ParticleEditorTab::Save()
{
	Particles::ParticleEmitter::SaveToFile(Particle->ParticleElements, ElementMaterials, CurrentSystemFile);
}

void ParticleEditorTab::Generate()
{
	Particle->Reset();
	for (auto* elem : SettingsButtons)
	{
		delete elem;
	}
	SettingsButtons.clear();
	for (auto* elem : GeneratedUI)
	{
		delete elem;
	}
	GeneratedUI.clear();
	SelectedElementText->SetPosition(Vector2(TabBackground->GetPosition() + TabBackground->GetUsedSize() * Vector2(0.5f, 1) - Vector2(0.1f, 0.15f)));
	for (unsigned int i = 0; i < Particle->ParticleElements.size(); i++)
	{
		Vector3 Color = i == SelectedElement ? UIColors[1] * 3 : UIColors[1];
		auto ButtonBox = new UIBox(true, TabBackground->GetPosition() + Vector2(0.05f, TabBackground->GetUsedSize().Y - 0.2f - (i / 10.f)));
		auto NewButton = new UIButton(true, 0, Color, this, 200 + i);
		NewButton->SetMinSize(Vector2(0.2f, 0));
		NewButton->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		ButtonBox->AddChild(NewButton);
		NewButton->SetPadding(0, 0, 0, 0.01f);
		auto RemoveButton = new UIButton(true, 0, UIColors[2], this, 300 + i);
		RemoveButton->SetMinSize(Vector2(0.09f, 0.16f) / 2.2f);
		RemoveButton->SetUseTexture(true, RemoveTexture);
		RemoveButton->SetPadding(0);
		ButtonBox->AddChild(RemoveButton);
		GeneratedUI.push_back(NewButton);
		GeneratedUI.push_back(RemoveButton);
		NewButton->AddChild(new UIText(0.5f, UIColors[2], "Element " + std::to_string(i), TabText));
	}
	if (Particle->ParticleElements.size())
	{
		SelectedElementText->SetText("Properties of 'Element " + std::to_string(SelectedElement) + "'");
	}
	else
	{
		SelectedElementText->SetText("Particle has no elements");
	}
	{
		auto NewButton = new UIButton(true,
			TabBackground->GetPosition() + Vector2(0.05f, TabBackground->GetUsedSize().Y - 0.2f - (Particle->ParticleElements.size() / 10.f)),
			UIColors[2], 
			this, 
			0);
		NewButton->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		GeneratedUI.push_back(NewButton);
		NewButton->SetMinSize(Vector2(0.2f, 0));
		NewButton->AddChild(new UIText(0.5f, 1 - UIColors[2], "Add element", TabText));
	}
	SettingsButtons.clear();
	if (ParticleSettingsScrollBox) delete ParticleSettingsScrollBox;
	ParticleSettingsScrollBox = new UIScrollBox(true, TabBackground->GetPosition() + Vector2(TabBackground->GetUsedSize().X * 0.5f - 0.1f, 0), true);
	float ScrollSize = TabBackground->GetUsedSize().Y - 0.2f;
	ParticleSettingsScrollBox->SetMaxSize(Vector2(0.6f, ScrollSize));
	ParticleSettingsScrollBox->SetMinSize(Vector2(0, ScrollSize));
	UIBox* ScrollBoxes[2] = { nullptr, nullptr };
	for (auto& i : ScrollBoxes)
	{
		i = new UIBox(false, 0);
		i->SetPadding(0);
		i->SetMinSize(Vector2(0.3f, ScrollSize));
		i->SetMaxSize(Vector2(0.3f, ScrollSize));
		ParticleSettingsScrollBox->AddChild(i);
	}

	if (SelectedElement < Particle->ParticleElements.size())
	{
		auto& SelectedParticle = Particle->ParticleElements[SelectedElement];
		auto NewText = (new UIText(0.7f, UIColors[2], "Velocity", TabText))->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
		ScrollBoxes[0]->AddChild(NewText);
		UIVectorField* NewVecField = new UIVectorField(0, SelectedParticle.Direction, this, 100, TabText);
		ScrollBoxes[0]->AddChild(NewVecField);
		SettingsButtons.push_back(NewVecField);

		NewText = (new UIText(0.7f, UIColors[2], "Velocity random", TabText))->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
		ScrollBoxes[0]->AddChild(NewText);
		NewVecField = new UIVectorField(0, SelectedParticle.DirectionRandom, this, 101, TabText);
		ScrollBoxes[0]->AddChild(NewVecField);
		SettingsButtons.push_back(NewVecField);

		NewText = (new UIText(0.7f, UIColors[2], "Size", TabText))->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
		ScrollBoxes[0]->AddChild(NewText);
		UITextField* NewTextField = new UITextField(Vector2(-0.1f, 0.1f), 0.2f, this, 102, TabText);
		ScrollBoxes[0]->AddChild(NewTextField);
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << SelectedParticle.Size;
		NewTextField->SetText(stream.str());
		NewTextField->SetMinSize(Vector2(0.265f, 0.05f));
		NewTextField->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		SettingsButtons.push_back(NewTextField);

		NewText = (new UIText(0.7f, UIColors[2], "Lifetime", TabText))->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
		ScrollBoxes[0]->AddChild(NewText);
		NewTextField = new UITextField(Vector2(-0.1f, 0.05f), 0.2f, this, 103, TabText);
		ScrollBoxes[0]->AddChild(NewTextField);
		stream = std::stringstream();
		stream << std::fixed << std::setprecision(2) << SelectedParticle.LifeTime;
		NewTextField->SetText(stream.str());
		NewTextField->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		NewTextField->SetMinSize(Vector2(0.265f, 0.05f));
		SettingsButtons.push_back(NewTextField);

		NewText = (new UIText(0.7f, UIColors[2], "Spawn delay", TabText))->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
		ScrollBoxes[0]->AddChild(NewText);
		NewTextField = new UITextField(Vector2(-0.1f, -0.2f), 0.2f, this, 104, TabText);
		ScrollBoxes[0]->AddChild(NewTextField);
		stream = std::stringstream();
		stream << std::fixed << std::setprecision(2) << SelectedParticle.SpawnDelay;
		NewTextField->SetText(stream.str());
		NewTextField->SetMinSize(Vector2(0.265f, 0.05f));
		NewTextField->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		SettingsButtons.push_back(NewTextField);

		NewText = (new UIText(0.7f, UIColors[2], "Spawn loops", TabText))->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
		ScrollBoxes[1]->AddChild(NewText);
		NewTextField = new UITextField(Vector2(-0.1f, -0.35f), 0.2f, this, 105, TabText);
		ScrollBoxes[1]->AddChild(NewTextField);
		NewTextField->SetText(std::to_string(SelectedParticle.NumLoops));
		NewTextField->SetMinSize(Vector2(0.265f, 0.05f));
		NewTextField->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		SettingsButtons.push_back(NewTextField);

		NewText = (new UIText(0.7f, UIColors[2], "Material", TabText))->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
		ScrollBoxes[1]->AddChild(NewText);
		NewTextField = new UITextField(Vector2(-0.1f, -0.5f), 0.2f, this, 106, TabText);
		ScrollBoxes[1]->AddChild(NewTextField);
		NewTextField->SetText(ElementMaterials[SelectedElement]);
		NewTextField->SetMinSize(Vector2(0.265f, 0.05f));
		NewTextField->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		SettingsButtons.push_back(NewTextField);
		
		NewText = (new UIText(0.7f, UIColors[2], "Position Random", TabText))->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
		ScrollBoxes[1]->AddChild(NewText);
		NewVecField = new UIVectorField(0, SelectedParticle.PositionRandom, this, 107, TabText);
		ScrollBoxes[1]->AddChild(NewVecField);
		SettingsButtons.push_back(NewVecField);

		NewText = (new UIText(0.7f, UIColors[2], "Force", TabText))->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
		ScrollBoxes[1]->AddChild(NewText);
		NewVecField = new UIVectorField(0, SelectedParticle.Force, this, 108, TabText);
		ScrollBoxes[1]->AddChild(NewVecField);
		SettingsButtons.push_back(NewVecField);

		NewText = (new UIText(0.7f, UIColors[2], "Start scale", TabText))->SetPadding(0.01f, 0.01f, 0.01f, 0.01f);
		ScrollBoxes[1]->AddChild(NewText);
		NewTextField = new UITextField(0, 0.2f, this, 109, TabText);
		stream = std::stringstream();
		stream << std::fixed << std::setprecision(2) << SelectedParticle.StartScale;
		ScrollBoxes[1]->AddChild(NewTextField);
		NewTextField->SetText(stream.str());
		NewTextField->SetMinSize(Vector2(0.265f, 0.05f));
		NewTextField->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		SettingsButtons.push_back(NewTextField);

		NewText = new UIText(0.7f, UIColors[2], "End scale", TabText);
		ScrollBoxes[1]->AddChild(NewText);
		NewTextField = new UITextField(0, 0.2f, this, 110, TabText);
		stream = std::stringstream();
		stream << std::fixed << std::setprecision(2) << SelectedParticle.EndScale;
		ScrollBoxes[1]->AddChild(NewTextField);
		NewTextField->SetText(stream.str());
		NewTextField->SetMinSize(Vector2(0.265f, 0.05f));
		NewTextField->SetBorder(UIBox::BorderType::Rounded, 0.5f);
		SettingsButtons.push_back(NewTextField);
	}
	for (auto* elem : SettingsButtons)
	{
		elem->IsVisible = TabBackground->IsVisible;
	}
	for (auto* elem : GeneratedUI)
	{
		elem->IsVisible = TabBackground->IsVisible;
	}
}

void ParticleEditorTab::OnButtonClicked(int Index)
{
	if (!TabBackground->IsVisible) return;
	UIBox* Button = nullptr;
	if (Index >= 100)
	{
		Button = SettingsButtons[(size_t)Index - 100];
	}
	try
	{
		if (Index == 0)
		{
			Particle->AddElement(Particles::ParticleElement(), Material::LoadMaterialFile("NONE"));
			ElementMaterials.push_back("");
			Generate();
			return;
		}
		if (Index >= 300 && Index < 400)
		{
			Particle->RemoveElement(Index - 300);
			ElementMaterials.erase(ElementMaterials.begin() + ((size_t)Index - 300));
			Generate();
			return;
		}
		if (Index >= 200 && Index < 300)
		{
			SelectedElement = Index - 200;
			Generate();
			return;
		}
		if (Index == 100)
		{
			Particle->ParticleElements[SelectedElement].Direction = ((UIVectorField*)Button)->GetValue();
			Generate();
			return;
		}
		if (Index == 101)
		{
			Particle->ParticleElements[SelectedElement].DirectionRandom = ((UIVectorField*)Button)->GetValue();
			Generate();
			return;
		}
		if (Index == 102)
		{
			Particle->ParticleElements[SelectedElement].Size = std::stof(((UITextField*)Button)->GetText());
			Generate();
			return;
		}
		if (Index == 103)
		{
			Particle->ParticleElements[SelectedElement].LifeTime = std::stof(((UITextField*)Button)->GetText());
			Generate();
			return;
		}
		if (Index == 104)
		{
			Particle->ParticleElements[SelectedElement].SpawnDelay = std::stof(((UITextField*)Button)->GetText());
			Generate();
			return;
		}
		if (Index == 105)
		{
			Particle->ParticleElements[SelectedElement].NumLoops = std::stoi(((UITextField*)Button)->GetText());
			Generate();
			return;
		}
		if (Index == 106)
		{
			Particle->SetMaterial(SelectedElement, Material::LoadMaterialFile(((UITextField*)Button)->GetText()));
			ElementMaterials[SelectedElement] = ((UITextField*)Button)->GetText();
			Generate();
			return;
		}
		if (Index == 107)
		{
			Particle->ParticleElements[SelectedElement].PositionRandom = ((UIVectorField*)Button)->GetValue();
			Generate();
			return;
		}
		if (Index == 108)
		{
			Particle->ParticleElements[SelectedElement].Force = ((UIVectorField*)Button)->GetValue();
			Generate();
			return;
		}
		if (Index == 109)
		{
			Particle->ParticleElements[SelectedElement].StartScale = std::stof(((UITextField*)Button)->GetText());
			Generate();
			return;
		}
		if (Index == 110)
		{
			Particle->ParticleElements[SelectedElement].EndScale = std::stof(((UITextField*)Button)->GetText());
			Generate();
			return;
		}
	}
	catch (std::exception& e)
	{
		Log::Print(e.what());
	}
}

ParticleEditorTab::~ParticleEditorTab()
{
}
#endif