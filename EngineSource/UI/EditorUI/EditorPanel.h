#pragma once
#if EDITOR
#include "UI/UIBackground.h"
#include "UI/UICanvas.h"
#include <UI/UIfwd.h>
#include <Engine/File/SaveData.h>

/**
* @brief
* A class representing a panel in the editor.
* 
* A panel can have multiple children that are either layed out as tabs, horizontally or vertically.
* 
*/
class EditorPanel : public UICanvas
{
protected:
	UIBackground* TabList = nullptr;
	Vector2 TotalScale;
	std::vector<UIButton*> Tabs;
	void AddTabButton(bool Selected, int Index, std::string Name, bool Closable);
	static void HandleDrag();
	bool TickPanelInternal();
	bool ClearParentOnDestruct = true;
public:
	bool UpdatePanelLayout = false;
	std::string ClassName;
	UIBackground* PanelMainBackground = nullptr;
	bool CanBeClosed = false;
	bool BackgroundVisible = true;
	static float StartPosition;
	static bool IsDraggingHorizontal;
	static EditorPanel* Dragged;

	size_t ActiveTab = 0;
	std::string Name;
	Vector2 Position;
	Vector2 Scale;
	float Size = 0.5f;
	bool Visible = true;
	EditorPanel* Parent = nullptr;
	bool Collapsed = false;
	void Collapse();
	void ClearParent(bool Delete);
	static void TickPanels();
	void ReSort();

	/**
	* @brief
	* Enum controlling the layout of the panel's children.
	*/
	enum class ChildrenType
	{
		Tabs,
		Horizontal,
		Vertical
	};

	struct DroppedItem
	{
		UICanvas* From = nullptr;
		int32_t TypeID = 0;
		std::string Path;
	};

	EditorPanel* GetAbsoluteParent();
	ChildrenType ChildrenAlign = ChildrenType::Tabs;
	std::vector<EditorPanel*> Children;

	EditorPanel(Vector2 Position, Vector2 Scale, std::string Name, std::string ClassName = "panel");
	EditorPanel(EditorPanel* Parent, std::string Name, std::string ClassName = "panel", size_t TabPosition = SIZE_MAX);

	/**
	* @brief
	* Function that handles Panel buttons.
	* 
	* @param Index
	* Index of the button that has been pressed.
	*/
	void HandlePanelButtons(int Index);
	void HandlePanelDrag(int Index);

	virtual void OnButtonClicked(int Index) override;
	virtual void OnButtonDragged(int Index) override;
	void UpdatePanel();
	virtual void Tick() override;
	void TickPanel();
	void OnPanelResized();
	void SetName(std::string Name);
	void AddPanelTab(EditorPanel* Panel);
	
	/**
	* This function will be called if the editor window has been resized.
	*/
	virtual void OnResized();

	/**
	* @brief
	* Adds a new tab to the panel.
	* 
	* @param NewTab
	* The tab that should be added.
	* 
	* @param Align
	* The align type of the panel.
	* 
	* @param TabPosition
	* The position index where the tab should be inserted.
	*/
	void AddTab(EditorPanel* NewTab, ChildrenType Align = ChildrenType::Tabs, size_t TabPosition = SIZE_MAX);


	/**
	* @brief
	* Calls the OnResized() function on all EditorPanel instances of the type T.
	*
	* @tparam T
	* The type of EditorPanel that should be updated.
	*/
	template<typename T>
	static void UpdateAllInstancesOf()
	{
		auto All = GetAllInstancesOf<T>();
		for (T* c : All)
		{
			c->OnResized();
		}
	}

	template<typename T>
	static std::vector<T*> GetAllInstancesOf()
	{
		std::vector<T*> All;
		for (UICanvas* c : Graphics::UIToRender)
		{
			if (dynamic_cast<T*>(c))
			{
				All.push_back(static_cast<T*>(c));
			}
		}
		return All;
	}

	void UpdateTabs();
	virtual ~EditorPanel();
	virtual void OnItemDropped(DroppedItem Info);
};
#endif