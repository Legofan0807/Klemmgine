#pragma once
#include <Objects/Components/Component.h>
#include <Rendering/Graphics.h>
#include <Rendering/Camera/Camera.h>

/**
* @brief
* A camera.
* 
* When used, the game view will be shown from this component's position.
* 
* @ingroup Components
*/
class CameraComponent : public Component
{
public:
	virtual void Begin() override;
	virtual void Update() override;
	virtual void Destroy() override;

	/// Sets the field of view of the camera in degrees.
	void SetFOV(float FOV);
	/// Gets the field of view of the camera in degrees.
	float GetFOV() const;
	CameraComponent();

	/**
	* @brief
	* Converts a 3d position in the world to a screen position.
	* 
	* @param WorldPos
	* The 3d position of the point.
	* 
	* @return
	* X and Y are the 3d position projected to the screen. Z is the depth of the point.
	*/
	Vector3 WorldToScreenPos(Vector3 WorldPos) const;

	/**
	* @brief
	* Gets a 3d direction vector from a position on screen.
	* 
	* @param ScreenPosition
	* The position of the point on screen.
	* 
	* @return
	* The direction in which the point faces.
	*/
	Vector3 GetForwardFromScreenPosition(Vector2 ScreenPosition) const;

	/**
	* @brief
	* Activates this camera on the main framebuffer.
	* 
	* If another camera is already active, it will be deactivated.
	*/
	void Use();

	/**
	* @brief
	* Returns true if this camera is currently being used.
	*/
	bool IsUsed() const;
protected:
	Camera ComponentCamera = Camera(2, Graphics::WindowResolution.X, Graphics::WindowResolution.Y, false);
};