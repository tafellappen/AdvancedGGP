#pragma once

#include <DirectXMath.h>
#include <vector>


class Transform
{
public:
	Transform();

	void MoveAbsolute(float x, float y, float z);
	void MoveRelative(float x, float y, float z);
	void Rotate(float p, float y, float r);
	void Scale(float x, float y, float z);

	void SetPosition(float x, float y, float z);
	void SetRotation(float p, float y, float r);
	void SetScale(float x, float y, float z);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	//void markTransformDirty();

private:
	// Raw transformation data
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 pitchYawRoll;
	DirectX::XMFLOAT3 scale;

	// World matrix and inverse transpose of the world matrix
	bool matricesDirty;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;

	// Helper to update both matrices if necessary
	void UpdateMatrices();

	//transformation hierarchies
	std::vector<Transform*> children;
	Transform* parent;

	int ChildCount();
	void AddChild(Transform* child);
	void RemoveChild(Transform* child);
	void SetParent(Transform* parentTransform);
	Transform* GetChild(int index);
	int GetIndexOfChild(Transform* child);
	Transform* GetParent();

	void MarkChildTransformsDirty();
};

