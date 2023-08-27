#include "Graphics.h"
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/Utility/Bloom.h>
#include <Rendering/Utility/SSAO.h>
#include <Rendering/Utility/CSM.h>
#include <UI/UIBox.h>

namespace Graphics
{
	float ResolutionScale = 1.0f;
	bool RenderShadows = true;
	bool SSAO = true;
	bool VSync = true;
	bool Bloom = true, FXAA = false;
	bool IsWireframe = false;

	Sun WorldSun;
	Fog WorldFog;
	int ShadowResolution = 2000;
	std::vector<Renderable*> ModelsToRender;
	std::vector<UICanvas*> UIToRender;
	Vector2 WindowResolution(1600, 900);
	unsigned int PCFQuality = 0;
	float AspectRatio = 16.0f / 9.0f;
	void SetWindowResolution(Vector2 NewResolution)
	{
		if (NewResolution == WindowResolution)
		{
			return;
		}
		for (FramebufferObject* o : AllFramebuffers)
		{
			if (o->UseMainWindowResolution)
			{
				o->GetBuffer()->ReInit((unsigned int)(NewResolution.X * ResolutionScale), (int)(NewResolution.Y * ResolutionScale));
			}
		}
		Graphics::MainCamera->ReInit(Graphics::MainCamera->FOV, NewResolution.X, NewResolution.Y, false);
		AspectRatio = NewResolution.X / NewResolution.Y;
		WindowResolution = NewResolution * ResolutionScale;
		SSAO::ResizeBuffer((unsigned int)(NewResolution.X * ResolutionScale), (unsigned int)(NewResolution.Y * ResolutionScale));
		Bloom::OnResized();
		UIBox::ForceUpdateUI();
	}
	float Gamma = 1;
	float ChrAbbSize = 0, Vignette = 0.1f;
	Camera* MainCamera;
	Shader* MainShader;
	Shader* ShadowShader;
	Shader* TextShader;
	Shader* UIShader;
	bool IsRenderingShadows = false;
	namespace UI
	{
		std::vector<ScrollObject*> ScrollObjects;
	}
	bool CanRenderText = true;
	namespace FBO
	{
		unsigned int SSAOBuffers[3];
		unsigned int ssaoColorBuffer;
		unsigned int ssaoFBO;
	}
	FramebufferObject* MainFramebuffer;
	std::vector<FramebufferObject*> AllFramebuffers;
	bool Light::operator==(Light b)
	{
		return Position == b.Position && Color == b.Color && Intensity == b.Intensity && Falloff == b.Falloff;
	}
}