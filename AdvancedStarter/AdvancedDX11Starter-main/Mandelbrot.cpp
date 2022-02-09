#include "Mandelbrot.h"

#include <iostream>

Mandelbrot::Mandelbrot(
	Microsoft::WRL::ComPtr<ID3D11Device> device, 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
	unsigned int windowWidth, 
	unsigned int windowHeight, 
	Camera* camera,
	Material* material
)
{
	this->device = device;
	this->context = context;
	//this->windowWidth = windowWidth;
	//this->windowHeight = windowHeight;
	this->camera = camera;
	this->material = material;

	this->fractalPlaneInfo.screenMidPosition = DirectX::XMFLOAT2(0, 0);
	this->fractalPlaneInfo.scale = STARTING_SCALE;

	movementSpeed = 0.5f;
	RetreiveShaders();
	//set up additional material data
	//we need to make sure the pixel shader can access the data that the compute shader is writing to
	this->material->AddPSTextureSRV("Pixels", computeTextureSRV);


	//CreateComputeShaderTexture();
	PostResize(windowWidth, windowHeight);
	InitRenderStates();
}

Mandelbrot::~Mandelbrot()
{
}

void Mandelbrot::Update(float dt)
{
	float speed = dt * movementSpeed;
	// Get the input manager instance
	Input& input = Input::GetInstance();

	// Speed up or down as necessary
	if (input.KeyDown(VK_SHIFT)) { speed *= 5; }
	if (input.KeyDown(VK_CONTROL)) { speed *= 0.1f; }

	// Movement
	if (input.KeyDown('X')) { fractalPlaneInfo.Zoom(-speed); }
	if (input.KeyDown(' ')) { fractalPlaneInfo.Zoom(speed); }
	if (input.KeyDown('W')) { fractalPlaneInfo.MoveScreenPos(0, speed); }
	if (input.KeyDown('S')) { fractalPlaneInfo.MoveScreenPos(0, -speed); }
	if (input.KeyDown('A')) { fractalPlaneInfo.MoveScreenPos(speed, 0); }
	if (input.KeyDown('D')) { fractalPlaneInfo.MoveScreenPos(-speed, 0); }
	
	//std::cout << windowWidth << ", " << windowHeight << std::endl;
	//std::cout << aspectRatio.x << ", " << aspectRatio.y << std::endl;
	//std::cout << speed << std::endl;
	//screenCenter = DirectX::XMFLOAT2(fractalSpaceInfo.screenCenterPosition.x, fractalSpaceInfo.screenCenterPosition.y);

}

void Mandelbrot::Draw(Camera* camera, Sky* sky)
{
	// Tell the material to prepare for a draw
	material->PrepareMaterial(camera, sky);

	//context->DrawInstancedIndirect()
	////SimpleVertexShader* vs = material->GetVS();
	//SimplePixelShader* ps = material->GetPS();


	//// Change to the sky-specific rasterizer state
	//context->RSSetState(mandelbrotRasterState.Get());
	//context->OMSetDepthStencilState(mandelbrotDepthState.Get(), 0);

	//// Set the sky shaders
	////skyVS->SetShader();
	//ps->SetShader();

	//// Give them proper data
	//skyVS->SetMatrix4x4("view", camera->GetView());
	//skyVS->SetMatrix4x4("projection", camera->GetProjection());
	//skyVS->CopyAllBufferData();

	//// Send the proper resources to the pixel shader
	//ps->SetShaderResourceView("skyTexture", skySRV);
	//ps->SetSamplerState("samplerOptions", samplerOptions);

	//// Set mesh buffers and draw
	//skyMesh->SetBuffersAndDraw(context);

	//// Reset my rasterizer state to the default
	//context->RSSetState(0); // Null (or 0) puts back the defaults
	//context->OMSetDepthStencilState(0, 0);
}


void Mandelbrot::RunComputeShader()
{
	fractalCS->SetShader();
	fractalCS->SetUnorderedAccessView("outputTexture", computeTextureUAV);


	fractalCS->SetFloat("height", windowHeight);
	fractalCS->SetFloat("width", windowWidth);
	fractalCS->SetFloat("scale", fractalPlaneInfo.scale);
	fractalCS->SetFloat2("center", fractalPlaneInfo.screenMidPosition);
	fractalCS->SetFloat2("aspectRatio", aspectRatio);
	fractalCS->SetInt("maxIter", 5000);
	fractalCS->CopyAllBufferData();
	
	// Dispatch the compute shader
	fractalCS->DispatchByThreads(windowWidth, windowHeight, 1);


	// Unbind the texture so we can use it later in draw
	fractalCS->SetUnorderedAccessView("outputTexture", 0);
}

Material* Mandelbrot::GetMaterial()
{
	return material;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Mandelbrot::GetSRV()
{
	return computeTextureSRV;
}


void Mandelbrot::PostResize(unsigned int windowWidth, unsigned int windowHeight)
{
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	//std::cout << aspectRatio.x << ", " << aspectRatio.y << std::endl;
	DirectX::XMFLOAT2 windowMiddle = DirectX::XMFLOAT2(windowWidth / 2, windowHeight / 2);
	aspectRatio = DirectX::XMFLOAT2(1, static_cast<float>(windowHeight) / static_cast<float>(windowWidth));

	//std::cout << windowWidth << "/" << windowHeight << std::endl;
	//std::cout << windowWidth / windowHeight << std::endl;


	CreateComputeShaderTexture();
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

void Mandelbrot::RetreiveShaders()
{
	AssetManager& mngr = AssetManager::GetInstance();
	fractalCS = mngr.GetComputeShader("FractalCS.cso");
	//fractalVS = mngr.GetVertexShader("FractalVS.cso");
	//fractalPS = mngr.GetPixelShader("FractalPS.cso");
}

void Mandelbrot::InitRenderStates()
{
	// Rasterizer to reverse the cull mode
	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT; // Draw the inside instead of the outside!
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rastDesc, mandelbrotRasterState.GetAddressOf());

	// Depth state so that we ACCEPT pixels with a depth == 1
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	device->CreateDepthStencilState(&depthDesc, mandelbrotDepthState.GetAddressOf());
}
