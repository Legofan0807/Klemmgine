#ifdef EDITOR
#pragma once
#include "UI/Default/UICanvas.h"
#include "UI/Default/TextRenderer.h"
#include <UI/UIBackground.h>
#include "UI/UIButton.h"
#include <UI/UITextField.h>
#include <UI/UIText.h>
#include <UI/EditorUI/EditorTab.h>
#include <UI/EditorUI/MaterialTemplateTab.h>
#include "MeshTab.h"
#include "ParticleEditorTab.h"
#include <Engine/TypeEnun.h>
#include <Objects/WorldObject.h>
#include <Rendering/Utility/Framebuffer.h>

#include <SDL.h>
#include <thread>

class EditorUI;
class UIVectorField;
class UITextField;
extern bool ChangedScene;

struct EditorClassesItem
{
	std::string Name;
	ObjectDescription Object = ObjectDescription("", 0);
	std::vector<EditorClassesItem> SubItems;
	bool IsFolder = false;
};

namespace Editor
{
	extern EditorUI* CurrentUI;
	extern bool DraggingTab;
	extern bool TabDragHorizontal;
	extern Vector2 DragMinMax;
}

class EditorUI : public UICanvas
{
public:
	EditorUI();
	void OnLeave(void(*ReturnF)());
	virtual void Tick() override;
	void GenUITextures();

	UIBackground* DragUI = nullptr;

	Vector3 UIColors[3] =
	{
		Vector3(0.1, 0.1, 0.11),	//Default background
		Vector3(0.07f),				//Dark background
		Vector3(1)					//Highlight color
		//Vector3(0.9, 0.9, 1),		//Default background
		//Vector3(0.6f),			//Dark background
		//Vector3(1, 0, 1)			//Highlight color};
	};
	TextRenderer* EngineUIText = new TextRenderer("Font.ttf", 90);

	enum CursorType
	{
		E_DEFAULT = 0,
		E_GRAB = 1,
		E_LOADING = 2,
		E_CROSS = 3,
		E_RESIZE_WE = 4,
		E_RESIZE_NS = 5,
		E_LAST_CURSOR = 6
	};

	CursorType CurrentCursor = E_DEFAULT;
	std::vector<unsigned int> Textures;

protected:
	SDL_Cursor* Cursors[6];
	float FPSUpdateTimer = 0;
	unsigned int DisplayedFPS = 60;

	std::set<std::string> CollapsedItems;
	std::vector<std::string> ObjectCategories;


	std::string ToShortString(float val);
	std::vector<EditorClassesItem> GetEditorUIClasses();
	struct ObjectListItem
	{
		ObjectListItem(std::string CategoryName, std::vector<ObjectListItem> Children, bool IsScene, bool IsCollapsed)
		{
			this->Name = CategoryName;
			this->Children = Children;
			this->IsScene = IsScene;
			this->IsCollapsed = IsCollapsed;
		}
		ObjectListItem(WorldObject* Object, int ListIndex)
		{
			this->Object = Object;
			if (Object) this->Name = Object->GetName();
			this->ListIndex = ListIndex;
		}

		bool IsCollapsed = false;
		std::vector<ObjectListItem> Children;
		std::string Name;
		WorldObject* Object = nullptr;
		int ListIndex = -1;
		bool IsScene = false;
	};
	std::vector<ObjectListItem> GetObjectList();


	std::vector<EditorClassesItem> GetContentsOfCurrentCPPFolder();
	std::string GetCurrentCPPPathString();

	UIBox* DropDownMenu = nullptr;
	Vector2 DropDownMenuPosition;
	friend WorldObject;
	std::vector<EditorClassesItem> CPPClasses;
	std::vector<size_t> CPPPath;
};
#endif