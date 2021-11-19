#pragma once
#include <DirectXMath.h>
#include <d3d11.h> // for ID3D11Device and ID3D11DeviceContext
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>


#include "SimpleShader.h"
#include "Material.h"
#include "Transform.h"

struct ParticleData
{
	float EmitTime;
	DirectX::XMFLOAT3 StartPosition;
};

class Emitter
{
public:
	Emitter();

	void Update();
	void Draw();
private:
	ParticleData particles; //how do arrays even work
	int firstLivingIndex;
	int firstDeadIndex;
	int livingCount;

	float particlesEmitPerSecond;
	float secBetweenParticleEmit;
	float timeSinceLastEmit;

	float particleLifetime;

	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;
	Transform transform;


};

