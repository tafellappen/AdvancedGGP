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
	MarkChildrenTransformsDirty();
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
	MarkChildrenTransformsDirty();
}

void Transform::Rotate(float p, float y, float r)
{
	pitchYawRoll.x += p;
	pitchYawRoll.y += y;
	pitchYawRoll.z += r;
	matricesDirty = true;
	MarkChildrenTransformsDirty();
}

void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
	matricesDirty = true;
	MarkChildrenTransformsDirty();
}

void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
	matricesDirty = true;
	MarkChildrenTransformsDirty();
}

void Transform::SetRotation(float p, float y, float r)
{
	pitchYawRoll.x = p;
	pitchYawRoll.y = y;
	pitchYawRoll.z = r;
	matricesDirty = true;
	MarkChildrenTransformsDirty();
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
	matricesDirty = true;
	MarkChildrenTransformsDirty();
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

		// Calculate the world matrix for this transform
		XMMATRIX wm = sc * rot * trans;

		//if there is a parent, need to also apply the world matrix from the parent
		if (parent)
		{
			XMFLOAT4X4 parentWorld = parent->GetWorldMatrix();
			wm *= XMLoadFloat4x4(&parentWorld);
		}

		//save
		XMStoreFloat4x4(&worldMatrix, wm);

		// Invert and transpose, too
		XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(wm)));

		// All set
		matricesDirty = false;
		MarkChildrenTransformsDirty();
	}
}

int Transform::ChildCount()
{
	return children.size();
}

void Transform::AddChild(Transform* child)
{
	if (!child) return; //verify pointer
	if (GetIndexOfChild(child) >= 0) return; //prevent adding duplicates

	// Add child to list
	children.push_back(child);
	child->parent = this;

	//mark child transforms out of date
	child->matricesDirty = true;
	child->MarkChildrenTransformsDirty();
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
		child->MarkChildrenTransformsDirty();
	}
}

void Transform::SetParent(Transform* newParent)
{
	//if this already has a parent, we need to make sure we unparent. So the parent of this transform also has to have this removed as a child
	if (this->parent)
	{
		this->parent->RemoveChild(this);
	}
	if (newParent) 
	{
		newParent->AddChild(this); //yeah this is big brain time (add child will both add "this" to the newParent child list and also set newParent as the parent to the child

	}
}

Transform* Transform::GetChild(unsigned int index)
{
	if (index >= children.size()) return 0;
	return children[index];
}

int Transform::GetIndexOfChild(Transform* child)
{
	if (!child) return -1; //verify pointer
	for (unsigned int i = 0; i < children.size(); i++)
	{
		if (child == children[i])
		{
			return (int)i;
		}
	}
	return -1; //not found
}

Transform* Transform::GetParent()
{
	return parent;
}

/// <summary>
/// Set the child of this transform as dirty, and recursively propogate through all children below it
/// </summary>
void Transform::MarkChildrenTransformsDirty()
{
	for (int i = 0; i < children.size(); i++)
	{
		children[i]->matricesDirty = true;
		children[i]->MarkChildrenTransformsDirty();
	}
}
