#include "GameEntity.h"

using namespace DirectX;

GameEntity::GameEntity(Mesh* mesh, Material* material)
{
	// Save the data
	this->mesh = mesh;
	this->material = material;
}

Mesh* GameEntity::GetMesh() { return mesh; }
Material* GameEntity::GetMaterial() { return material; }
Transform* GameEntity::GetTransform() { return &transform; }


void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Camera* camera, Sky* sky)
{
	// Tell the material to prepare for a draw
	material->PrepareMaterial(&transform, camera, sky);

	// Draw the mesh
	mesh->SetBuffersAndDraw(context);
}
