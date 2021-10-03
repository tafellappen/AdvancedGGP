#pragma once


#include <DirectXMath.h>
#include <d3d11.h> // for ID3D11Device and ID3D11DeviceContext
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include "Sky.h"
#include "GameEntity.h"
#include "Lights.h"

#include "AssetManager.h"

#include <memory>

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

	void Render(std::shared_ptr<Camera> camera,	int lightCount);

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

	//int lightCount;

	Mesh* lightMesh;
	SimpleVertexShader* lightVS;
	SimplePixelShader* lightPS;

	void DrawPointLights(std::shared_ptr<Camera> camera, int lightCount);

};

