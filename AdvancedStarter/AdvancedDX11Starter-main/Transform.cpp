#include "Transform.h"

using namespace DirectX;


Transform::Transform()
{
	// Start with an identity matrix and basic transform data
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());

	position = XMFLOAT3(0, 0, 0);
	pitchYawRoll = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);

	// No need to recalc yet
	matricesDirty = false;
	parent = 0;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	matricesDirty = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	// Create a direction vector from the params
	// and a rotation quaternion
	XMVECTOR movement = XMVectorSet(x, y, z, 0);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));

	// Rotate the movement by the quaternion
	XMVECTOR dir = XMVector3Rotate(movement, rotQuat);

	// Add and store, and invalidate the matrices
	XMStoreFloat3(&position, XMLoadFloat3(&position) + dir);
	matricesDirty = true;
}

void Transform::Rotate(float p, float y, float r)
{
	pitchYawRoll.x += p;
	pitchYawRoll.y += y;
	pitchYawRoll.z += r;
	matricesDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
	matricesDirty = true;
}

void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
	matricesDirty = true;
}

void Transform::SetRotation(float p, float y, float r)
{
	pitchYawRoll.x = p;
	pitchYawRoll.y = y;
	pitchYawRoll.z = r;
	matricesDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
	matricesDirty = true;
}

DirectX::XMFLOAT3 Transform::GetPosition() { return position; }

DirectX::XMFLOAT3 Transform::GetPitchYawRoll() { return pitchYawRoll; }

DirectX::XMFLOAT3 Transform::GetScale() { return scale; }


DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	UpdateMatrices();
	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	UpdateMatrices();
	return worldMatrix;
}
//
//void Transform::markTransformDirty()
//{
//	matricesDirty = true;
//}

void Transform::UpdateMatrices()
{
	// Are the matrices out of date (dirty)?
	if (matricesDirty)
	{
		// Create the three transformation pieces
		XMMATRIX trans = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
		XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
		XMMATRIX sc = XMMatrixScalingFromVector(XMLoadFloat3(&scale));

		// Combine and store the world
		XMMATRIX wm = sc * rot * trans;
		XMStoreFloat4x4(&worldMatrix, wm);

		// Invert and transpose, too
		XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(wm)));

		// All set
		matricesDirty = false;
	}
}

int Transform::ChildCount()
{
	return children.size();
}

void Transform::AddChild(Transform* child)
{
	if (!child) return; //verify pointer
	if (GetIndexOfChild(child) <= -1) return; //prevent adding duplicates

	// Add child to list
	children.push_back(child);
	child->parent = this;

	//mark child transforms out of date
	child->matricesDirty = true;
}

void Transform::RemoveChild(Transform* child)
{
	if (!child) return; //verify pointer

	auto childToRemove = std::find(children.begin(), children.end(), child);
	if (childToRemove != children.end())
	{
		children.erase(childToRemove);
		child->parent = 0;

		child->matricesDirty = true;
		child->MarkChildTransformsDirty();
	}
}

void Transform::SetParent(Transform* newParent)
{
	if (this->parent)
	{
		this->parent->RemoveChild(this);
	}
	if (newParent) 
	{
		parent->AddChild(this); //yeah this is big brain time

	}
}

Transform* Transform::GetChild(int index)
{
	return children[index];
}

int Transform::GetIndexOfChild(Transform* child)
{
	for (int i = 0; i < children.size(); i++)
	{
		if (child == children[i])
		{
			return i;
		}
	}
	return -1; //return -1 if child is not found
}

Transform* Transform::GetParent()
{
	return parent;
}

void Transform::MarkChildTransformsDirty()
{
	for (int i = 0; i < children.size(); i++)
	{
		children[i]->matricesDirty = true;
	}
}
