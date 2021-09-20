#pragma once

#include <d3d11.h> // for ID3D11Device and ID3D11DeviceContext
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include "Sky.h"
#include "GameEntity.h"
#include "Lights.h"

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
		const std::vector<GameEntity*>& entities,
		const std::vector<Light*>& lights		
	);

	void PostResize(
		unsigned int windowWidth,
		unsigned int windowHeight,
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV,
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDS
	);

	void Render(Camera* camera);

private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV;
	unsigned int windowWidth;
	unsigned int windowHeight;
	Sky* sky;
	const std::vector<GameEntity*>& entities;
	const std::vector<Light*>& lights;

	void DrawPointLights(Camera* camera);
};

