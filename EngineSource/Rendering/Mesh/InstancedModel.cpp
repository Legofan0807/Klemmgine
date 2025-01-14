#if !SERVER
#include "InstancedModel.h"
#include <filesystem>
#include <fstream>
#include <Rendering/ShaderManager.h>
#include <Rendering/Mesh/ModelGenerator.h>
#include <Engine/Log.h>
#include <Rendering/Graphics.h>
#include <Engine/File/Assets.h>
#include <Engine/Stats.h>
#include <GL/glew.h>
#include <Engine/EngineError.h>


InstancedModel::InstancedModel(std::string Filename)
{
	if (std::filesystem::exists(Filename))
	{
		ModelData.LoadModelFromFile(Filename);
		CastShadow = ModelData.CastShadow;
		TwoSided = ModelData.TwoSided;
		for (size_t i = 0; i < ModelData.Elements.size(); i++)
		{
			InstancedMesh* NewMesh = new InstancedMesh(ModelData.Elements[i].Vertices, ModelData.Elements[i].Indices,
				Material::LoadMaterialFile(ModelData.Elements[i].ElemMaterial));
			Meshes.push_back(NewMesh);
		}
		NonScaledSize = ModelData.CollisionBox;
		ConfigureVAO();
	}
	else
	{
		Log::Print("Model does not exist: " + Filename);
	}
}


InstancedModel::~InstancedModel()
{
	for (InstancedMesh* m : Meshes)
	{
		delete m;
	}
	Meshes.clear();
	if (MatBuffer != -1)
		glDeleteBuffers(1, &MatBuffer);
}

void InstancedModel::Render(Camera* WorldCamera, bool MainFrameBuffer, bool TransparencyPass)
{
	if (MatModel.empty())
	{
		return;
	}

	if (TwoSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
	}
	glm::mat4 ModelView;
	ModelView = WorldCamera->getView() * MatModel[0];
	glm::mat4 InvModelView = glm::transpose(glm::inverse(ModelView));
	glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);
	for (int i = 0; i < Meshes.size(); i++)
	{
		if (Meshes[i]->RenderContext.Mat.IsTranslucent != TransparencyPass) continue;
		Shader* CurrentShader = Meshes[i]->RenderContext.GetShader();
		CurrentShader->Bind();
		glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_projection"), 1, GL_FALSE, &WorldCamera->GetProjection()[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_invmodelview"), 1, GL_FALSE, &InvModelView[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_viewpro"), 1, GL_FALSE, &WorldCamera->GetViewProjection()[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(CurrentShader->GetShaderID(), "u_view"), 1, GL_FALSE, &WorldCamera->getView()[0][0]);
		Meshes.at(i)->Render(CurrentShader, MainFrameBuffer);
		Stats::DrawCalls++;
	}
}

size_t InstancedModel::AddInstance(Transform T)
{
	Instances.push_back(T);
	return Instances.size() - 1;
}

bool InstancedModel::RemoveInstance(size_t Inst)
{
	if (Inst > Instances.size()) return false;
	Instances.erase(Instances.begin() + Inst);
	return true;
}

void InstancedModel::ConfigureVAO()
{
	MatModel.clear();
	MatModel.reserve(Instances.size());
	for (Transform &T : Instances)
	{
		glm::mat4 Inst = glm::mat4(1.f);

		Size = (NonScaledSize * T.Scale * 0.025f);
		Inst = glm::translate(Inst, (glm::vec3)T.Position);
		Inst = glm::rotate(Inst, T.Rotation.Y, glm::vec3(0, 1, 0));
		Inst = glm::rotate(Inst, T.Rotation.Z, glm::vec3(0, 0, 1));
		Inst = glm::rotate(Inst, T.Rotation.X, glm::vec3(1, 0, 0));

		MatModel.push_back(glm::scale(Inst, (glm::vec3)(T.Scale) * 0.025f));
	}
	if(MatBuffer != -1)
		glDeleteBuffers(1, &MatBuffer);

	MatBuffer = -1;

	if (MatModel.empty())
	{
		return;
	}

	glGenBuffers(1, &MatBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);
	glBufferData(GL_ARRAY_BUFFER, MatModel.size() * sizeof(glm::mat4), &MatModel[0], GL_STATIC_DRAW);
	for (int i = 0; i < Meshes.size(); i++)
	{
		unsigned int VAO = Meshes[i]->MeshVertexBuffer->VAO;
		glBindVertexArray(VAO);
		// vertex attributes
		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)0);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);

		glBindVertexArray(0);
		Meshes[i]->SetInstances(Instances);
	}
}

void InstancedModel::SimpleRender(Shader* UsedShader)
{
	UsedShader->Bind();
	if (TwoSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
	}
	glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);
	for (InstancedMesh* m : Meshes)
	{
		m->SimpleRender(UsedShader);
	}
	Stats::DrawCalls++;
}
#endif