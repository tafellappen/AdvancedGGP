#pragma once


#include <DirectXMath.h>
#include <d3d11.h> // for ID3D11Device and ID3D11DeviceContext
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include "Sky.h"
#include "GameEntity.h"
#include "Lights.h"
#include "Emitter.h"
#include "SceneManager.h"
#include "Mandelbrot.h"
//
//#include "AssetManager.h"

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

//// Specifically for mandelbrot
//struct MandelPSPerFrameData
//{
//	DirectX::XMFLOAT4 Color2;
//};

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
		SimplePixelShader* lightPS,
		std::vector<std::shared_ptr<Emitter>> particleEmitters,
		std::shared_ptr<Mandelbrot> mandelbrot
	);

	~Renderer();

	void PostResize(
		unsigned int windowWidth,
		unsigned int windowHeight,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV
	);

	void UpdateLightVec(std::vector<Light> lights);

	void Render(Camera* camera,	int lightCount, float totalTime, SceneState currentSceneState);
	//void StandardSceneRender(const float  color[4], Camera* camera, int lightCount, float totalTime, SceneState currentSceneState);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSceneColorsSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSceneNormalsSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSceneDepthsSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetRefractionSilhouetteSRV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetFinalCompositeSRV();

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

	std::shared_ptr<Mandelbrot> mandelbrot;

	//multiple render targets things - for refraction
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneColorsRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneNormalsRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneDepthsRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> refractionSilhouetteRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> finalCompositeRTV;
	//SRVs also seem to go best here. repo just has all of these things in arrays instead. probably nicer for adding more but i cant even go watch Platinum End right now so we cant have nice things anyway
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneColorsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneNormalsSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneDepthsSRV;	//these all used for imgui i think? maybe? idk but word vomiting into code comments helps me think
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> refractionSilhouetteSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> finalCompositeSRV;

	//other misc refraction things
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> refractionSilhouetteDepthState;
	bool refractionFromNormalMap;
	float indexOfRefraction;
	float refractionScale;

	// Per-frame constant buffers and data
	Microsoft::WRL::ComPtr<ID3D11Buffer> psPerFrameConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsPerFrameConstantBuffer;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> psMandePerFrameConstantBuffer;
	PSPerFrameData psPerFrameData;
	VSPerFrameData vsPerFrameData;
	//MandelPSPerFrameData psMandelPerFrameData;
	DirectX::XMFLOAT4 testColor;


	// Overall ambient for non-pbr shaders
	DirectX::XMFLOAT3 ambientNonPBR;

	//particle things
	std::vector<std::shared_ptr<Emitter>> particleEmitters;
	Microsoft::WRL::ComPtr<ID3D11BlendState> particleBlendAdditive;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> particleDepthState;


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

