#include "Renderer.h"

Renderer::Renderer(
	Microsoft::WRL::ComPtr<ID3D11Device> device, 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain, 
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV, 
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV, 
	unsigned int windowWidth, 
	unsigned int windowHeight, 
	Sky* sky, 
	const std::vector<GameEntity*>& entities, 
	const std::vector<Light*>& lights)
{
	this->device = device;
	this->context = context;
	this->swapChain = swapChain;
	this->backBufferRTV = backBufferRTV;
	this->depthBufferDSV = depthBufferDSV;
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	this->sky = sky;
	this->entities = &entities;
	this->lights = &lights;
}

Renderer::~Renderer()
{
	for (auto& e : *entities) delete e;
	for (auto& l : *lights) delete l;
	delete sky;
	sky = nullptr;
}

void Renderer::PostResize(unsigned int windowWidth, unsigned int windowHeight, Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV, Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV)
{
	this->backBufferRTV = backBufferRTV;
	this->depthBufferDSV = depthBufferDSV;
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
}

void Renderer::Render(Camera* camera)
{
}

void Renderer::DrawPointLights(Camera* camera)
{
}
