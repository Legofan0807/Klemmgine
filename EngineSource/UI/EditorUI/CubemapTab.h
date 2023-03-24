#if 0
#pragma once
#include "EditorTab.h"
#include <UI/UITextField.h>
#include "MaterialFunctions.h"
#include <Engine/Save.h>

class FramebufferObject;
class Model;
class UIText;
class UIScrollBox;

class CubemapTab : public EditorTab
{
public:
	UIBackground* PreviewWindow = nullptr;

	CubemapTab(Vector3* UIColors, TextRenderer* Renderer);
	void Tick() override;
	void Load(std::string File) override;
	void Save() override;
	void OnButtonClicked(int Index) override;
	void Generate();
	virtual ~CubemapTab();
protected:
	std::vector<std::string> DisplayNames = { "Right face", "Left face", "Upper face", "Lower face", "Front face", "Back face" };
	std::vector<std::string> Cubenames = { "right", "left", "up", "down", "front", "back" };

	std::vector<UITextField*> SideFields;
	SaveGame* SaveFile = nullptr;
	void UpdatePreviewModel();
	FramebufferObject* PreviewBuffer = nullptr;
	Model* PreviewModel = nullptr;
	Transform CameraTransform;
	TextRenderer* Renderer;
	UIText* TabName;
	UIBox* CubemapSidesBox = nullptr;
	std::string InitialName;
};
#endif