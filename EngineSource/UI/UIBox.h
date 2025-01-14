#if !SERVER
#pragma once
#include <Math/Vector.h>
#include <cmath>
#include <UI/Default/ScrollObject.h>

class UIScrollBox;
class UIButton;

/**
* @defgroup UI
* 
* @brief
* Functions/classes related to UI.
*/

/**
* @brief
* UIBox class. Parent class for all UI elements.
* 
* The UIBox itself is not visible.
* 
* C# equivalent: Engine.UI.UIBox
* 
* @ingroup UI
*/
class UIBox
{
public:
	/// True if this box should be rendered, false if not.
	bool IsVisible = true;

	/**
	* @brief
	* Children align for UIBox.
	*/

	enum class Align
	{
		/// Align from lowest to highest. if Orientation = Horizontal, this means from left to right.
		Default,
		/// Centered align. will act like Align.Default, but children will be put in the center of the box.
		Centered,
		/// Align from highest to lowest. if Orientation = Horizontal, this means from right to left.
		Reverse
	};

	/**
	* @brief
	* Describes the border of a UIBox.
	*/
	enum class BorderType
	{
		/// No border.
		None = 0,
		/// Rounded border.
		Rounded = 1,
		/// Darkened edge around the box.
		DarkenedEdge = 2
	};
	/**
	 * @brief
	 * Controls the orientation for all children.
	 * 
	 * If Orientation is Horizontal, the children of this UIBox will be aligned horizontally.
	 * 
	 * ```
	 * Example:
	 * 
	 *  ____________________________________    ____________________________________
	 * | _______   _______                  |  | _______                            |
	 * ||Child 1| |Child 2|                 |  ||Child 1|                           |
	 * ||_______| |_______|                 |  ||_______|  Parent box               |
	 * |                                    |  | _______   Orientation: Vertical    |
	 * |      Parent box                    |  ||Child 2|                           |
	 * |      Orientation: Horizontal       |  ||_______|                           |
	 * |____________________________________|  |____________________________________|
	 * ```
	 */
	enum class Orientation
	{
		/// Vertical orientation for all children.
		Horizontal,
		/// Horizontal orientation for all children.
		Vertical
	};

	/**
	* @brief
	* Describes the way size should be calculated for a UIBox.
	*/
	enum class SizeMode
	{
		/**
		* @brief
		* Default value. Size should be relative to the screen.
		*
		* A box with the position x=-1, y=-1 (bottom left corner) and a size of x=1, y=2 will always fill half the screen.
		*
		* A box where `size x` = `size y` will only be square if the screen itself is square.
		*/
		ScreenRelative = 0,

		/**
		* @brief
		* Size should be relative to the aspect ratio/pixels.
		*
		* A box where `size x` = `size y` is guaranteed to be square.
		*
		* A AspectRelative box with the size x=1, x=1 has the same size as a ScreenRelative box with the size x=1/AspectRatio, y=1.
		*/
		AspectRelative = 1,

		/**
		 * @brief
		 * Size should be relative to the screen resolution.
		 *
		 * A box where `size x` = `size y` is guaranteed to be square.
		 * No matter the screens resolution, the box will always have the same size in pixels.
		 */
		PixelRelative = 2,
	};

	virtual std::string GetAsString();
	void DebugPrintTree(uint8_t Depth = 0);

	/**
	* @brief
	* Sets the size mode of the minimum and maximum size value of this UIBox.
	*/
	UIBox* SetSizeMode(SizeMode NewMode);
	BorderType BoxBorder = BorderType::None;
	SizeMode PaddingSizeMode = SizeMode::ScreenRelative;
	float BorderRadius = 0;
	Align HorizontalBoxAlign = Align::Default;
	Align VerticalBoxAlign = Align::Reverse;

	/**
	 * @brief
	 * Constructs UIBox with the given orientation and position.
	 */
	UIBox(Orientation BoxOritentation, Vector2 Position);
	/**
	* @brief
	* When deleted, a UIBox will also delete all children.
	*/
	virtual ~UIBox();

	virtual void OnAttached();
	void InvalidateLayout();
	UIBox* AddChild(UIBox* NewChild);
	UIBox* GetAbsoluteParent();

	/**
	 * @brief
	 * Sets the horizontal align for all children.
	 * 
	 * ```
	 * Example
	 *  ____________________________________    ____________________________________
	 * | _______   _______                  |  |         _______   _______          |
	 * ||Child 1| |Child 2|                 |  |        |Child 1| |Child 2|         |
	 * ||_______| |_______|                 |  |        |_______| |_______|         |
	 * |                                    |  |                                    |
	 * | Parent box                         |  | Parent box                         |
	 * | Horizontal Align: Default          |  | Horizontal Align: Centered         |
	 * |____________________________________|  |____________________________________|
	 * 
	 * ```
	 * @return
	 * A reference to this %UIBox.
	 */
	UIBox* SetHorizontalAlign(Align NewAlign);

	/**
	 * @brief
	 * Sets the horizontal align for all children.
	 * 
	 * Notes:
	 * - The default vertical align is Align.Reverse, not Align.Default.
	 * - **Align::Reverse aligns boxes from the top down (from 1 to -1)** while Align.Default aligns boxes from the bottom up (from -1 to 1)
	 *   The reason for this is that this corresponds to the way horizontal align works.
	 *   (Default is from -1 to 1 - left to right, Reverse 1 to -1, right to left)
	 * 
	 * ```
	 * Example
	 *  ____________________________________    ____________________________________
	 * | _______   _______                  |  | Vertical Align: Default            |
	 * ||Child 1| |Child 2|                 |  | Parent box                         |
	 * ||_______| |_______|                 |  |                                    |
	 * |                                    |  | _______   _______                  |
	 * | Parent box                         |  ||Child 1| |Child 2|                 |
	 * | Vertical Align: Reverse            |  ||_______| |_______|                 |
	 * |____________________________________|  |____________________________________|
	 * ```
	 * @return
	 * A reference to this %UIBox.
	 */
	UIBox* SetVerticalAlign(Align NewAlign);
	static void DrawAllUIElements();
	
	/**
	* @brief
	* Clears and deletes all children from this UIBox.
	*/
	void DeleteChildren();

	/**
	* @brief
	* Returns true if the element is visible.
	* 
	* Even if this box is visible, one of it's parents might not be. This function accurately returns if the UIBox will be drawn.
	*/
	bool IsVisibleInHierarchy();
	
	/**
	 * @brief
	 * Sets the maximum size this UIBox can occupy.
	 *
	 * @param NewMaxSize
	 * The new maximum size the UIBox should occupy.
	 *
	 * @return
	 * A reference to this %UIBox.
	 */
	UIBox* SetMaxSize(Vector2 NewMaxSize);
	Vector2 GetMaxSize() const;

	/**
	 * @brief
	 * Sets the minimum size this UIBox can occupy.
	 * 
	 * @param NewMinSize
	 * The new minimum size the UIBox should occupy.
	 * 
	 * @return
	 * A reference to this %UIBox.
	 */
	UIBox* SetMinSize(Vector2 NewMinSize);

	/// Gets the minimum size this UIBox can occupy
	Vector2 GetMinSize() const;

	/**
	 * @brief
	 * Sets the position of the UIBox.
	 * 
	 * @param NewPosition
	 * The new position of the box, where -1, -1 is the bottom left corner of the screen and 1, 1 is the top right corner.
	 */
	UIBox* SetPosition(Vector2 NewPosition);

	/**
	 * @brief
	 * Gets the position of the UIBox.
	 *
	 * @return
	 * The position of the box, where -1, -1 is the bottom left corner of the screen and 1, 1 is the top right corner.
	 */
	Vector2 GetPosition() const;

	/**
	 * @brief
	 * Sets the padding of a UIBox in each direction.
	 *
	 * Padding works like margin in CSS.
	 *
	 * @return
	 * A reference to this %UIBox.
	 */
	UIBox* SetPadding(float Up, float Down, float Left, float Right);

	/**
	 * @brief
	 * Sets the padding of a UIBox, in all directions.
	 *
	 * Padding works like margin in CSS.
	 *
	 * @return
	 * A pointer to this UIBox.
	 */
	UIBox* SetPadding(float AllDirs);

	/**
	* @brief
	* If true, this box will try to fill up the entire space aligned opposite of the parent's orientation.
	*/
	UIBox* SetTryFill(bool NewTryFill);

	/**
	* @brief
	* Sets the size mode of the padding of this box.
	*/
	UIBox* SetPaddingSizeMode(SizeMode NewSizeMode);
	
	/**
	* @brief
	* Sets the orientation of this UIBox.
	*/
	UIBox* SetOrientation(Orientation NewOrientation);

	Orientation GetOrientation() const;
	bool GetTryFill() const;
	friend UIScrollBox;
	virtual void OnChildClicked(int Index);

	/**
	 * @brief
	 * Sets the border type of the UIBox.
	 * 
	 * This only has an effect on the classes UIButton, UIBackground and UITextField.
	 * 
	 * @param NewBorderType
	 * The type of the border. See UIBox::BorderType for more info.
	 * 
	 * @param Size
	 * The size of the border.
	 * 
	 * @return
	 * A pointer to this %UIBox.
	 */
	UIBox* SetBorder(UIBox::BorderType NewBorderType, float Size);
	static void ForceUpdateUI();
	static void InitUI();
	static unsigned int* GetUITextures();
	static void RedrawUI();
	static void ClearUI();
	static void UpdateUI();
	
	/**
	* @brief
	* Returns true if the mouse cursor is above this box.
	* 
	* If another box is above this box, it will not affect the return value of this function.
	* To check what box is actually hovered right now, use UI::HoveredBox
	*/
	bool IsHovered() const;
	
	/**
	 * @brief
	 * Gets the used size of the UIBox, the size that the UIBox occupies.
	 *
	 * @return
	 * The used size of the box.
	 */
	virtual Vector2 GetUsedSize();
	ScrollObject* CurrentScrollObject = nullptr;

	bool IsChildOf(UIBox* Parent);
	bool HasMouseCollision = false;
	void UpdateSelfAndChildren();
	std::vector<UIBox*> GetChildren();

	/**
	* @brief
	* Sets the render order index of this box.
	* 
	* A box with a lower order index will be drawn first.
	*/
	void SetRenderOrderIndex(size_t OrderIndex);
	/// Gets the render order index of this box. See @ref UIBox::SetRenderOrderIndex()
	size_t GetRenderOrderIndex();

	static std::vector<ScrollObject*> ScrollObjects;

	struct RedrawBox
	{
		Vector2 Min;
		Vector2 Max;

		static bool IsBoxOverlapping(const RedrawBox& BoxA, const RedrawBox& BoxB);
	};

	/**
	* @brief
	* Adds the area around this element to be redrawn the next frame.
	*/
	void RedrawElement();
	static void RedrawArea(RedrawBox Box);

	/**
	* @brief
	* Gets the parent of this UIBox.
	*/
	UIBox* GetParent();
	Vector2 GetLeftRightPadding() const;
	void GetPaddingScreenSize(Vector2& UpDown, Vector2& LeftRight) const;

	/**
	* @brief
	* Converts a pixel size to a screen size.
	* 
	* Useful for calculating positions.
	*/
	static Vector2 PixelSizeToScreenSize(Vector2 PixelSize);

	static float DpiScale;

protected:
	bool ShouldBeTicked = true;
	bool TryFill = false;
	virtual void Update();
	virtual void Draw();
	virtual void Tick();
	virtual void UpdateTickState();
	void UpdateHoveredState();
	
	Vector2 Position;
	Vector2 OffsetPosition = NAN;
	Vector2 MaxSize = Vector2(2, 2);
	Vector2 MinSize = Vector2(0, 0);

	float UpPadding = 0;
	float DownPadding = 0;
	float RightPadding = 0;
	float LeftPadding = 0;
	Vector2 Size;
	SizeMode BoxSizeMode = SizeMode::ScreenRelative;

	std::vector<UIBox*> Children;
	UIBox* Parent = nullptr;
	void UpdateScale();
	void UpdatePosition();
private:
	void SetOffsetPosition(Vector2 NewPos);
	float GetVerticalOffset();
	float GetHorizontalOffset();
	void DrawThisAndChildren(RedrawBox Area);
	Orientation ChildrenOrientation = Orientation::Horizontal;
	bool PrevIsVisible = true;
	static std::vector<RedrawBox> RedrawBoxes;
};

namespace UI
{
	extern UIBox* HoveredBox;
	extern UIBox* NewHoveredBox;
}
#endif