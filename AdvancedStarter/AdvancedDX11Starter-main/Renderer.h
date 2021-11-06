#pragma once


#include <DirectXMath.h>
#include <d3d11.h> // for ID3D11Device and ID3D11DeviceContext
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include "Sky.h"
#include "GameEntity.h"
#include "Lights.h"

#include "AssetManager.h"

// This needs to match the expected per-frame vertex shader data
struct VSPerFrameData
{
	DirectX::XMFLOAT4X4 ViewMatrix;
	DirectX::XMFLOAT4X4 ProjectionMatrix;
};

// This needs to match the expected per-frame pixel shader data
struct PSPerFrameData
{
	Light Lights[MAX_LIGHTS];
	int LightCount;
	DirectX::XMFLOAT3 CameraPosition;
	int TotalSpecIBLMipLevels;
	DirectX::XMFLOAT3 AmbientNonPBR;
};

class Renderer
{
public:
	Renderer(
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV,
		unsigned int windowWidth,
		unsigned int windowHeight,
		Sky* sky,
		std::vector<GameEntity*> entities,
		std::vector<Light> lights,
		Mesh* lightMesh,
		SimpleVertexShader* lightVS,
		SimplePixelShader* lightPS
	);

	~Renderer();

	void PostResize(
		unsigned int windowWidth,
		unsigned int windowHeight,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV
	);

	void UpdateLightVec(std::vector<Light> lights);

	void Render(Camera* camera,	int lightCount);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSceneColorsSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSceneNormalsSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSceneDepthsSRV();

private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV;
	unsigned int windowWidth;
	unsigned int windowHeight;
	Sky* sky;
	std::vector<GameEntity*> entities;
	std::vector<Light> lights;

	//multiple render targets things - for refraction
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneColorsRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneNormalsRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneDepthsRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> refractionSilhouetteRTV;
	//SRVs also seem to go best here. repo just has all of these things in arrays instead. probably nicer for adding more but i cant even go watch Platinum End right now so we cant have nice things anyway
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneColorsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneNormalsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneDepthsSRV;	//these all used for imgui i think? maybe? idk but word vomiting into code comments helps me think
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> refractionSilhouetteSRV;

	//other misc refraction things
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> refractionSilhouetteDepthState;
	bool useRefractionSilhouette;
	bool refractionFromNormalMap;
	float indexOfRefraction;
	float refractionScale;

	// Per-frame constant buffers and data
	Microsoft::WRL::ComPtr<ID3D11Buffer> psPerFrameConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsPerFrameConstantBuffer;
	PSPerFrameData psPerFrameData;
	VSPerFrameData vsPerFrameData;


	// Overall ambient for non-pbr shaders
	DirectX::XMFLOAT3 ambientNonPBR;

	//int lightCount;

	Mesh* lightMesh;
	SimpleVertexShader* lightVS;
	SimplePixelShader* lightPS;

	void DrawPointLights(Camera* camera, int lightCount);

	void CreateRenderTarget(
		unsigned int width,
		unsigned int height,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv,
		DXGI_FORMAT colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM
	);

};

