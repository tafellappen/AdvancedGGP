#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "SimpleShader.h"
#include "Camera.h"
#include "Lights.h"

#include <memory>

class Material
{
public:
	Material(
		std::shared_ptr<SimpleVertexShader> vs,
		std::shared_ptr<SimplePixelShader> ps,
		DirectX::XMFLOAT4 color, 
		float shininess, 
		DirectX::XMFLOAT2 uvScale, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> albedo, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normals, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughness, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metal, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	~Material();

	void PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> cam);

	std::shared_ptr<SimpleVertexShader> GetVS() { return vs; }
	std::shared_ptr<SimplePixelShader> GetPS() { return ps; }

	void SetVS(std::shared_ptr<SimpleVertexShader> vs) { this->vs = vs; }
	void SetPS(std::shared_ptr<SimplePixelShader> ps) { this->ps = ps; }

private:
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;

	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT4 color;
	float shininess;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> albedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
};

