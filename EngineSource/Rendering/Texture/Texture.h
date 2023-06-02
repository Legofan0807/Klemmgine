#pragma once
#include <Math/Vector.h>

namespace Texture
{
	struct Texture
	{
		unsigned int TextureID = 0;
		unsigned int References = 0;
		std::string TexturePath;

		Texture(unsigned int TextureID,
		unsigned int References ,
		std::string TexturePath)
		{
			this->TextureID = TextureID;
			this->References = References;
			this->TexturePath = TexturePath;
		}
	};

	struct TextureData
	{
		std::vector<Vector3> Pixels;
		unsigned int ResolutionX = 0, ResolutionY = 0;
	};

	extern std::vector<Texture> Textures;

	unsigned int LoadTexture(std::string File);
	unsigned int CreateTexture(TextureData T);

	unsigned int LoadCubemapTexture(std::vector<std::string> Files);

	void UnloadTexture(unsigned int TextureID);
}