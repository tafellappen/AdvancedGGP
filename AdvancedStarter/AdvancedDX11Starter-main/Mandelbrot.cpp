#include "Mandelbrot.h"

Mandelbrot::Mandelbrot(
	Microsoft::WRL::ComPtr<ID3D11Device> device, 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
	unsigned int windowWidth, 
	unsigned int windowHeight, 
	Camera* camera
)
{
	this->device = device;
	this->context = context;
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	this->camera = camera;

	CreateComputeShaderTexture();
}

void Mandelbrot::Update()
{
}

void Mandelbrot::PostResize(unsigned int windowWidth, unsigned int windowHeight)
{
	CreateComputeShaderTexture();
}

void Mandelbrot::RunComputeShader()
{
	SimpleComputeShader* fractalCS = AssetManager::GetInstance().GetComputeShader("FractalCS.cso");

	fractalCS->SetShader();
	fractalCS->SetUnorderedAccessView("outputTexture", computeTextureUAV);

	//fractalCS->SetInt("iterations", noiseInterations);
	//fractalCS->SetFloat("persistence", noisePersistance);
	//fractalCS->SetFloat("scale", noiseScale);
	//fractalCS->SetFloat("offset", noiseOffset);
	fractalCS->CopyAllBufferData();

	// Dispatch the compute shader
	fractalCS->DispatchByThreads(windowWidth, windowHeight, 1);

	// Unbind the texture so we can use it later in draw
	fractalCS->SetUnorderedAccessView("outputTexture", 0);
}

void Mandelbrot::CreateComputeShaderTexture()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> fractalTexture;
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = windowWidth;
	texDesc.Height = windowHeight;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&texDesc, 0, fractalTexture.GetAddressOf());

	// Create the SRV using a default description (passing in null below)
	device->CreateShaderResourceView(fractalTexture.Get(), 0, &computeTextureSRV);

	// Create the UAV that treats this resource as a 2D texture
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = texDesc.Format;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	device->CreateUnorderedAccessView(fractalTexture.Get(), &uavDesc, &computeTextureUAV);
}
