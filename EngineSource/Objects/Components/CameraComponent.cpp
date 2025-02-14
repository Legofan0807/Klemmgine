#include "CameraComponent.h"
#include <Engine/Stats.h>
#include <Rendering/Framebuffer.h>
#include <Engine/Log.h>

void CameraComponent::Begin()
{
}

void CameraComponent::Update()
{
#if !SERVER
	Vector3 ParentRotation = GetParent()->GetTransform().Rotation;
	Transform ParentTransform = Transform(GetParent()->GetTransform().Position,
		Vector3(0, ParentRotation.Y, -ParentRotation.X).DegreesToRadians(), GetParent()->GetTransform().Scale * RelativeTransform.Scale);
	ComponentCamera.Position = Vector3::TranslateVector(RelativeTransform.Position, ParentTransform);
	ParentRotation = GetParent()->GetTransform().Rotation + RelativeTransform.Rotation;
	ComponentCamera.SetRotation(ParentRotation);
	ComponentCamera.Update();
#endif
}

void CameraComponent::Destroy()
{
#if !SERVER
	for (FramebufferObject* f : Graphics::AllFramebuffers)
	{
		if (f->FramebufferCamera == &ComponentCamera)
		{
			f->FramebufferCamera = nullptr;
		}
	}
#endif
}

Vector3 CameraComponent::WorldToScreenPos(Vector3 WorldPos) const
{
#if !SERVER
	glm::vec4 pos = ComponentCamera.GetViewProjection() * glm::vec4(glm::vec3(WorldPos), 1.0f);
	return Vector3(Vector2(pos.x, pos.y) / pos.z, pos.z);
#endif
	return 0;
}

Vector3 CameraComponent::GetForwardFromScreenPosition(Vector2 ScreenPosition) const
{
#if !SERVER
	return ComponentCamera.ForwardVectorFromScreenPosition(ScreenPosition.X, ScreenPosition.Y);
#endif
	return Vector3();
}


void CameraComponent::SetFOV(float FOV)
{
#if !SERVER
	ComponentCamera.ReInit(glm::radians(FOV) * 2, Graphics::WindowResolution.X, Graphics::WindowResolution.Y, false);
#endif
}

float CameraComponent::GetFOV() const
{
	return glm::degrees(ComponentCamera.FOV) / 2;
}

CameraComponent::CameraComponent()
{
}

void CameraComponent::Use()
{
#if !SERVER
	if (!IsInEditor)
	{
		Graphics::MainFramebuffer->FramebufferCamera = &ComponentCamera;
		Graphics::MainCamera = &ComponentCamera;
	}
#endif
}

bool CameraComponent::IsUsed() const
{
#if !SERVER
	return Graphics::MainCamera == &ComponentCamera;
#endif
	return false;
}
