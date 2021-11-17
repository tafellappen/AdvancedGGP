#include "Material.h"


Material::Material(
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
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerClamp//,
	//bool refractive = false
)
{
	this->vs = vs;
	this->ps = ps;
	this->color = color;
	this->shininess = shininess;
	this->albedoSRV = albedo;
	this->normalSRV = normals;
	this->roughnessSRV = roughness;
	this->metalSRV = metal;
	this->sampler = sampler;
	this->samplerClamp = samplerClamp;
	this->uvScale = uvScale;
	this->refractive = false; //defaulting this to false for now because clean code is a myth in week 12
	//this->refractive = refractive;
}


Material::~Material()
{
}

void Material::PrepareMaterial(Transform* transform, Camera* cam, Sky* sky)
{
	// Turn shaders on
	vs->SetShader();
	ps->SetShader();

	// Set vertex shader data
	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("worldInverseTranspose", transform->GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("view", cam->GetView());
	vs->SetMatrix4x4("projection", cam->GetProjection());
	vs->SetFloat2("uvScale", uvScale);
	vs->CopyAllBufferData();

	// Set pixel shader data
	ps->SetFloat4("Color", color); 
	ps->SetFloat("Shininess", shininess);
	ps->CopyBufferData("perMaterial");

	// Set SRVs
	ps->SetShaderResourceView("AlbedoTexture", albedoSRV);
	ps->SetShaderResourceView("NormalTexture", normalSRV);
	ps->SetShaderResourceView("RoughnessTexture", roughnessSRV);
	ps->SetShaderResourceView("MetalTexture", metalSRV);

	//set IBL SRV's
	ps->SetShaderResourceView("BrdfLookUpMap", sky->GetIblBrdfLookup());
	ps->SetShaderResourceView("IrradianceIBLMap", sky->GetIblIrradianceCubeMap());
	ps->SetShaderResourceView("SpecularIBLMap", sky->GetIblConvolvedSpecular());

	// Set sampler
	ps->SetSamplerState("BasicSampler", sampler);
	ps->SetSamplerState("ClampSampler", samplerClamp);
}

void Material::SetPerMaterialDataAndResources(bool copyToGPUNow, Sky* sky)
{
	//set vertex shader per-material variables
	vs->SetFloat2("uvScale", uvScale);
	if (copyToGPUNow)
	{
		vs->CopyBufferData("perMaterial");
	}

	//pixel shader per-mat variables
	ps->SetFloat4("Color", color);
	ps->SetFloat("Shininess", shininess);
	if (copyToGPUNow)
	{
		ps->CopyBufferData("perMaterial");
	}

	//wait, why is this different from "prepare material"
	// Set SRVs
	ps->SetShaderResourceView("AlbedoTexture", albedoSRV);
	ps->SetShaderResourceView("NormalTexture", normalSRV);
	ps->SetShaderResourceView("RoughnessTexture", roughnessSRV);
	ps->SetShaderResourceView("MetalTexture", metalSRV);

	//set IBL SRV's
	ps->SetShaderResourceView("BrdfLookUpMap", sky->GetIblBrdfLookup());
	ps->SetShaderResourceView("IrradianceIBLMap", sky->GetIblIrradianceCubeMap());
	ps->SetShaderResourceView("SpecularIBLMap", sky->GetIblConvolvedSpecular());

	// Set sampler
	ps->SetSamplerState("BasicSampler", sampler);
	ps->SetSamplerState("ClampSampler", samplerClamp);

}

void Material::SetRefractive(bool refractive)
{
	this->refractive = refractive;
}

bool Material::GetRefractive()
{
	return refractive;
}
