#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "SimpleShader.h"
#include "Camera.h"
#include "Lights.h"
#include "Sky.h"

class Material
{
public:
	Material(
		SimpleVertexShader* vs, 
		SimplePixelShader* ps, 
		DirectX::XMFLOAT4 color, 
		float shininess, 
		DirectX::XMFLOAT2 uvScale,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> albedo, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normals, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughness, 
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metal, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerClamp/*,
		bool refractive = false*/
		);
	~Material();
	//for normal mesh drawing
	void PrepareMaterial(Transform* transform, Camera* cam, Sky* sky);
	//used for drawing with no meshes (just for drawing the compute shader results directly to the screen right now - not using it as a mesh texture right now)
	void PrepareMaterial(Camera* cam, Sky* sky);
	void SetPerMaterialDataAndResources(bool copyToGPUNow, Sky* sky);

	SimpleVertexShader* GetVS() { return vs; }
	SimplePixelShader* GetPS() { return ps; }

	void SetVS(SimpleVertexShader* vs) { this->vs = vs; }
	void SetPS(SimplePixelShader* ps) { this->ps = ps; }

	void AddPSTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddVSTextureSRV(std::string shaderName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);

	//i really really really do not have the time or patience to update material definitions and the asset manager
	void SetRefractive(bool refractive);
	bool GetRefractive();

private:
	SimpleVertexShader* vs;
	SimplePixelShader* ps;

	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT4 color;
	float shininess;
	bool refractive;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> albedoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughnessSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> metalSRV;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> psOtherTextureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> vsOtherTextureSRVs;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerClamp;
};

