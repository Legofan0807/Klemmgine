#include "BakedLighting.h"
#include <GL/glew.h>
#include <Math/Vector.h>
#include <Engine/EngineRandom.h>
#include <Engine/EngineError.h>
#include <iostream>
#include <Math/Collision/Collision.h>
#include <World/Graphics.h>
#include <Objects/Components/MeshComponent.h>
#include <Rendering/Mesh/ModelGenerator.h>
#include <thread>
#include <Engine/Log.h>
#include <World/Assets.h>
#include <filesystem>
#include <Engine/Scene.h>
#include <fstream>
#include <Engine/Application.h>

unsigned int BakedLighting::LightTexture = 0;
float BakedLighting::LightmapScaleMultiplier = 1;
std::atomic<bool> BakedLighting::FinishedBaking = false;
uint64_t BakedLighting::LightmapResolution = 100;
constexpr uint8_t NUM_CHANNELS = 1;

Vector3 LightmapScale = 200.0f;

void BakedLighting::Init()
{
	std::byte* Texture = new std::byte[LightmapResolution * LightmapResolution * LightmapResolution * NUM_CHANNELS]();

	for (uint64_t i = 0; i < LightmapResolution * LightmapResolution * LightmapResolution * NUM_CHANNELS; i += NUM_CHANNELS)
	{
		for (uint8_t c = 0; c < NUM_CHANNELS; c++)
		{
			Texture[i + c] = std::byte(255);
		}
	}

	if (LightTexture)
	{
		glDeleteTextures(1, &LightTexture);
	}

	glGenTextures(1, &LightTexture);
	glBindTexture(GL_TEXTURE_3D, LightTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, LightmapResolution, LightmapResolution, LightmapResolution, 0, GL_RED, GL_UNSIGNED_BYTE, Texture);
	delete[] Texture;
	
}

void BakedLighting::BindToTexture()
{
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, LightTexture);
}

int64_t BakedLighting::GetLightTextureSize()
{
	return LightmapResolution;
}

constexpr uint8_t BKDAT_FILE_VERSION = 1;

#if EDITOR

namespace Bake
{
	std::vector<Collision::CollisionMesh> Meshes;
	Vector3 BakeScale;

	Vector3 BakeMapToPos(uint64_t TextureElement)
	{
		int64_t x = TextureElement % BakedLighting::LightmapResolution;
		int64_t y = (TextureElement / BakedLighting::LightmapResolution) % BakedLighting::LightmapResolution;
		int64_t z = TextureElement / (BakedLighting::LightmapResolution * BakedLighting::LightmapResolution);

		return Vector3(x, y, z) - (BakedLighting::LightmapResolution / 2);
	}


	std::byte* Texture = nullptr;

#define NUM_CHUNK_SPLITS 2
	std::atomic<float> ThreadProgress[NUM_CHUNK_SPLITS * NUM_CHUNK_SPLITS * NUM_CHUNK_SPLITS];

	void BakeSection(int64_t x, int64_t y, int64_t z, size_t ThreadID)
	{
		const uint64_t SECTION_SIZE = BakedLighting::LightmapResolution / NUM_CHUNK_SPLITS;
		const float ProgressPerPixel = 1.0f / (SECTION_SIZE * SECTION_SIZE * SECTION_SIZE);
		for (uint64_t i = 0; i < BakedLighting::LightmapResolution * BakedLighting::LightmapResolution * BakedLighting::LightmapResolution; i++)
		{
			int64_t px = i % BakedLighting::LightmapResolution;
			int64_t py = (i / BakedLighting::LightmapResolution) % BakedLighting::LightmapResolution;
			int64_t pz = i / (BakedLighting::LightmapResolution * BakedLighting::LightmapResolution);
			px /= SECTION_SIZE;
			py /= SECTION_SIZE;
			pz /= SECTION_SIZE;

			if (px != x || py != y || pz != z)
			{
				continue;
			}


			Vector3 Pos = BakeMapToPos(i);
			Pos = Pos / BakedLighting::LightmapResolution;
			Pos = Pos * BakeScale;
			float Intensity = BakedLighting::GetLightIntensityAt(Pos.X, Pos.Y, Pos.Z);
			Texture[i] = std::byte(Intensity * 255);
			ThreadProgress[ThreadID] += ProgressPerPixel;
		}
	}
}

namespace Bake
{
	inline Collision::HitResponse BakeRayTrace(const glm::vec3& orig, const glm::vec3& end, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C)
	{
		glm::vec3 dir = end;
		glm::vec3 E1 = C - A;
		glm::vec3 E2 = B - A;
		glm::vec3 N = glm::cross(E1, E2);
		float det = -glm::dot(dir, N);
		float invdet = 1.0 / det;
		glm::vec3 AO = orig - A;
		glm::vec3 DAO = glm::cross(AO, dir);
		float u = dot(E2, DAO) * invdet;
		float v = -dot(E1, DAO) * invdet;
		float t = dot(AO, N) * invdet;
		if ((t >= 0.0 && u >= 0.0 && v >= 0.0 && (u + v) <= 1.0))
			return Collision::HitResponse(true, orig + end * t, normalize(N), t);
		else return Collision::HitResponse();
	}
}

float BakedLighting::GetLightIntensityAt(int64_t x, int64_t y, int64_t z)
{
	const float TraceDistance = 2500;
	glm::vec3 StartPos = glm::vec3((float)x, (float)y, (float)z);
	StartPos = StartPos + glm::vec3(Bake::BakeScale / LightmapResolution / 2);
	glm::vec3 Direction = (glm::vec3)Graphics::WorldSun.Direction;
	Collision::HitResponse r;
	r.t = 0;

	for (auto& mesh : Bake::Meshes)
	{
		Collision::Box BroadPhaseBox = Collision::Box(
			mesh.SpherePosition.X - mesh.SphereCollisionSize, mesh.SpherePosition.X + mesh.SphereCollisionSize,
			mesh.SpherePosition.Y - mesh.SphereCollisionSize, mesh.SpherePosition.Y + mesh.SphereCollisionSize,
			mesh.SpherePosition.Z - mesh.SphereCollisionSize, mesh.SpherePosition.Z + mesh.SphereCollisionSize)
			.TransformBy(Transform(mesh.WorldPosition, Vector3(), Vector3(1)));
		if (!Collision::LineCheckForAABB(BroadPhaseBox,
			StartPos, StartPos + Direction * glm::vec3(TraceDistance)).Hit)
		{
			continue;
		}
		for (size_t i = 0; i < mesh.Indices.size(); i += 3)
		{
			glm::vec3* CurrentTriangle[3] = 
			{ 
				&mesh.Vertices[mesh.Indices[i]].Position,
				&mesh.Vertices[mesh.Indices[i + 2]].Position,
				&mesh.Vertices[mesh.Indices[i + 1]].Position 
			};
			Collision::HitResponse newR
				= (Bake::BakeRayTrace(StartPos, Direction, *CurrentTriangle[0], *CurrentTriangle[1], *CurrentTriangle[2]));
			if (newR.Hit && newR.t > r.t)
			{
				r = newR;
			}
		}
	}
	return r.Hit ? 1 - std::min(r.t * TraceDistance / 10, 1.0f) : 1;
}

std::byte Sample3DArray(std::byte* Arr, int64_t x, int64_t y, int64_t z)
{
	if (x < 0 || x >= BakedLighting::LightmapResolution
		|| y < 0 || y >= BakedLighting::LightmapResolution
		|| z < 0 || z >= BakedLighting::LightmapResolution)
	{
		return std::byte(255);
	}

	return Bake::Texture[x * BakedLighting::LightmapResolution * BakedLighting::LightmapResolution + y * BakedLighting::LightmapResolution + z];
}

void BakedLighting::BakeCurrentSceneToFile()
{
	Bake::Meshes.clear();

	for (WorldObject* i : Objects::AllObjects)
	{
		for (Component* c : i->GetComponents())
		{
			if (dynamic_cast<MeshComponent*>(c))
			{
				ModelGenerator::ModelData m = dynamic_cast<MeshComponent*>(c)->GetModelData();
				if (!m.CastShadow)
					continue;
				Vector3 InvertedRotation = (i->GetTransform().Rotation + c->RelativeTransform.Rotation);
				InvertedRotation = Vector3(-InvertedRotation.Z, InvertedRotation.Y, -InvertedRotation.X);
				Transform ModelTransform = (Transform(Vector3::TranslateVector(c->RelativeTransform.Location, i->GetTransform()),
					Vector3() - InvertedRotation.DegreesToRadiants(),
					c->RelativeTransform.Scale * 0.025f * i->GetTransform().Scale));

				Bake::Meshes.push_back(Collision::CollisionMesh(m.GetMergedVertices(), m.GetMergedIndices(), ModelTransform));
			}
		}
	}

	new std::thread([]() {

		Application::Timer BakeTimer;
		BakeLogMessages.clear();
		BakeLog("Baking lightmap for scene " + FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene));
		for (auto& i : Bake::ThreadProgress)
		{
			i = 0;
		}
		Bake::Texture = new std::byte[LightmapResolution * LightmapResolution * LightmapResolution * NUM_CHANNELS]();

		std::vector<std::thread*> BakeThreads;

		Collision::Box sbox;

		for (auto& i : Bake::Meshes)
		{
			if (!i.CanOverlap)
			{
				continue;
			}
			for (auto& vert : i.Vertices)
			{
				if (vert.Position.x > sbox.maxX)
				{
					sbox.maxX = vert.Position.x;
				}
				if (vert.Position.y > sbox.maxY)
				{
					sbox.maxY = vert.Position.y;
				}
				if (vert.Position.z > sbox.maxZ)
				{
					sbox.maxZ = vert.Position.z;
				}
				if (vert.Position.x < sbox.minX)
				{
					sbox.minX = vert.Position.x;
				}
				if (vert.Position.y < sbox.minY)
				{
					sbox.minY = vert.Position.y;
				}
				if (vert.Position.z < sbox.minZ)
				{
					sbox.minZ = vert.Position.z;
				}
			}
		}

		Bake::BakeScale = (sbox.GetExtent() * 2) + sbox.GetCenter();
		BakeLog("Calculated scene bounding box: " + std::to_string(Bake::BakeScale.Length()));

		Bake::BakeScale = std::max(Bake::BakeScale.X, std::max(Bake::BakeScale.Y, Bake::BakeScale.Z)) * LightmapScaleMultiplier;
		BakeLog("Baking with scale: " + std::to_string(Bake::BakeScale.X));

		size_t Thread3DArraySize = NUM_CHUNK_SPLITS;

		size_t ThreadID = 0;
		for (size_t x = 0; x < Thread3DArraySize; x++)
		{
			for (size_t y = 0; y < Thread3DArraySize; y++)
			{
				for (size_t z = 0; z < Thread3DArraySize; z++)
				{
					BakeThreads.push_back(new std::thread(Bake::BakeSection, x, y, z, ThreadID++));
				}
			}
		}

		BakeLog("Invoked " + std::to_string(BakeThreads.size()) + " threads");

		size_t i = 0;
		for (std::thread* t : BakeThreads)
		{
			t->join();
			BakeLog("Thread " + std::to_string(++i) + "/" + std::to_string(BakeThreads.size()) + " is done.");
		}

		BakeLog("Finished baking lightmap.");
		BakeLog("Bake took " + std::to_string((int)BakeTimer.TimeSinceCreation()) + " seconds.");

		// Simple RLE for lightmap compression.
		std::byte* TexPtr = Bake::Texture;

		std::string BakFile = Assets::GetAsset(FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene) + ".bkdat");
		if (!std::filesystem::exists(BakFile))
		{
			BakFile = FileUtil::GetFilePathWithoutExtension(Scene::CurrentScene) + ".bkdat";
		}

		std::ofstream OutFile = std::ofstream(BakFile);

		OutFile << (int)BKDAT_FILE_VERSION;



		struct RLEelem
		{
			std::byte Value = std::byte(1);
			size_t Length = 0;
		};

		RLEelem Current;
		std::vector<RLEelem> Elements;

		BakeLog("Compressing lightmap...");
		for (size_t i = 0; i < LightmapResolution * LightmapResolution * LightmapResolution * NUM_CHANNELS; i++)
		{
			std::byte CurrentVal = Bake::Texture[i];
			if (Current.Value != CurrentVal)
			{
				if (Current.Length != 0)
				{
					Elements.push_back(Current);
				}
				Current.Value = CurrentVal;
				Current.Length = 0;
			}
			Current.Length++;
		}

		size_t ElemsSize = Elements.size();
		OutFile << " " << ElemsSize << " " << LightmapResolution << std::endl;
		OutFile << Bake::BakeScale.ToString() << std::endl;

		size_t TotalLength = 0;

		BakeLog("Compressed lightmap with " + std::to_string(ElemsSize) + " RLE elements.");
		for (auto& i : Elements)
		{
			std::byte Val = i.Value;
			size_t Length = i.Length;
			TotalLength += Length;
			OutFile << Length << " ";
			OutFile << (int)Val << std::endl;
		}

		BakeLog("Encoded voxels: " + std::to_string(TotalLength));

		delete[] Bake::Texture;
		Bake::Meshes.clear();
		BakedLighting::FinishedBaking = true;
		});
}


float BakedLighting::GetBakeProgress()
{
	float Progress = 0;
	for (float i : Bake::ThreadProgress)
	{
		Progress += i;
	}
	return Progress / (NUM_CHUNK_SPLITS * NUM_CHUNK_SPLITS * NUM_CHUNK_SPLITS);
}


#endif

void BakedLighting::LoadBakeFile(std::string BakeFile)
{
	std::string File;
	if (!std::filesystem::exists(BakeFile))
	{
		File = Assets::GetAsset(BakeFile + ".bkdat");
	}
	else
	{
		File = BakeFile;
	}
	if (!std::filesystem::exists(File))
	{
		Log::Print("Could not find .bkdat file: " + BakeFile, Log::LogColor::Red);
		BakedLighting::Init();
		return;
	}

	std::ifstream InFile = std::ifstream(File);

	int FileVer = 0;
	Log::Print("Loading lightmap: " + File);
	InFile >> FileVer;
	if (BKDAT_FILE_VERSION != FileVer)
	{
		Log::Print("FILE VERSION MISMATCH: " + std::to_string(FileVer));
		BakedLighting::Init();
		return;
	}


	size_t FileLength = 0;
	InFile >> FileLength;
	InFile >> LightmapResolution;
	size_t Iterator = 0;

	float ScaleX, ScaleY, ScaleZ;

	InFile >> ScaleX;
	InFile >> ScaleY;
	InFile >> ScaleZ;

	LightmapScale = Vector3(ScaleX, ScaleY, ScaleZ);

	uint8_t* Texture = new uint8_t[LightmapResolution * LightmapResolution * LightmapResolution * NUM_CHANNELS * 2]();


	for (size_t i = 0; i < FileLength; i++)
	{
		int Value = 0;
		size_t Length = 0;
		InFile >> Length;
		InFile >> Value;
		for (size_t i = 0; i < Length; i++)
		{
			Texture[Iterator] = Value;
			Iterator++;
		}
	}


	glDeleteTextures(1, &LightTexture);
	glGenTextures(1, &LightTexture);
	glBindTexture(GL_TEXTURE_3D, LightTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, LightmapResolution, LightmapResolution, LightmapResolution, 0, GL_RED, GL_UNSIGNED_BYTE, Texture);
	delete[] Texture;
}

Vector3 BakedLighting::GetLightMapScale()
{
	return LightmapScale;
}

std::vector<std::string> BakedLighting::BakeLogMessages;

void BakedLighting::BakeLog(std::string Msg)
{
	BakeLogMessages.push_back(Msg);
}

std::vector<std::string> BakedLighting::GetBakeLog()
{
	return BakeLogMessages;
}