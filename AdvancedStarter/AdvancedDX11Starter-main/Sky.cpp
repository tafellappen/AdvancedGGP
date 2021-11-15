#include "Sky.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"

#include "AssetManager.h"

using namespace DirectX;

Sky::Sky(
	const wchar_t* cubemapDDSFile, 
	Mesh* mesh, 
	SimpleVertexShader* skyVS, 
	SimplePixelShader* skyPS, 
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions, 
	Microsoft::WRL::ComPtr<ID3D11Device> device, 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	// Save params
	this->skyMesh = mesh;
	this->device = device;
	this->context = context;
	this->samplerOptions = samplerOptions;
	this->skyVS = skyVS;
	this->skyPS = skyPS;

	// Init render states
	InitRenderStates();

	// Load texture
	CreateDDSTextureFromFile(device.Get(), cubemapDDSFile, 0, skySRV.GetAddressOf());
}

Sky::Sky(
	const wchar_t* right, 
	const wchar_t* left, 
	const wchar_t* up, 
	const wchar_t* down, 
	const wchar_t* front, 
	const wchar_t* back, 
	Mesh* mesh,
	SimpleVertexShader* skyVS,
	SimplePixelShader* skyPS,
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	// Save params
	this->skyMesh = mesh;
	this->device = device;
	this->context = context;
	this->samplerOptions = samplerOptions;
	this->skyVS = skyVS;
	this->skyPS = skyPS;

	// Init render states
	InitRenderStates();

	// Create texture from 6 images
	skySRV = CreateCubemap(right, left, up, down, front, back);

	// Set up IBL
	IBLCreateIrradianceMap();
	IBLCreateConvolvedSpecularMap();
	IBLCreateBRDFLookUpTexture();
}

Sky::~Sky()
{
}

void Sky::Draw(Camera* camera)
{
	// Change to the sky-specific rasterizer state
	context->RSSetState(skyRasterState.Get());
	context->OMSetDepthStencilState(skyDepthState.Get(), 0);

	// Set the sky shaders
	skyVS->SetShader();
	skyPS->SetShader();

	// Give them proper data
	skyVS->SetMatrix4x4("view", camera->GetView());
	skyVS->SetMatrix4x4("projection", camera->GetProjection());
	skyVS->CopyAllBufferData();

	// Send the proper resources to the pixel shader
	skyPS->SetShaderResourceView("skyTexture", skySRV);
	skyPS->SetSamplerState("samplerOptions", samplerOptions);

	// Set mesh buffers and draw
	skyMesh->SetBuffersAndDraw(context);

	// Reset my rasterizer state to the default
	context->RSSetState(0); // Null (or 0) puts back the defaults
	context->OMSetDepthStencilState(0, 0);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::GetIblIrradianceCubeMap()
{
	return iblIrradianceCubeMap;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::GetIblConvolvedSpecular()
{
	return iblConvolvedSpecular;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::GetIblBrdfLookup()
{
	return iblBrdfLookup;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::GetEnvironmentMap()
{
	return skySRV;
}

int Sky::GetMipLevelCount()
{
	return mipLevelCount;
}

void Sky::InitRenderStates()
{
	// Rasterizer to reverse the cull mode
	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT; // Draw the inside instead of the outside!
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rastDesc, skyRasterState.GetAddressOf());

	// Depth state so that we ACCEPT pixels with a depth == 1
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	device->CreateDepthStencilState(&depthDesc, skyDepthState.GetAddressOf());
}

void Sky::IBLCreateIrradianceMap()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> irrMapFinalTexture = {}; //i think those brackets are right??? im not sure

	//  STEP 1------------------------------------------------------------------------------------------------
	// Create the final irradiance cube texture
	D3D11_TEXTURE2D_DESC textDesc = {};
	textDesc.Width				= iblCubeMapFaceSize;			// make it square
	textDesc.Height				= iblCubeMapFaceSize;			// make it square
	textDesc.ArraySize			= 6;							// a cube has 6 sides
	textDesc.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // will be used as both
	textDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;	// basic texture format
	textDesc.MipLevels			= 1;							// no mip chain needed
	textDesc.MiscFlags			= D3D11_RESOURCE_MISC_TEXTURECUBE; // its a cube map
	textDesc.SampleDesc.Count	= 1;							//can't be zero
	device->CreateTexture2D(&textDesc, 0, irrMapFinalTexture.GetAddressOf());


	//  STEP 2------------------------------------------------------------------------------------------------
	// Create SRV for the irradiance texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;	// sample as cube map
	srvDesc.TextureCube.MipLevels = 1;							// only 1 mip level
	srvDesc.TextureCube.MostDetailedMip = 0;					// accessing the only mip
	srvDesc.Format = textDesc.Format;							// same format as texture
	device->CreateShaderResourceView(
		irrMapFinalTexture.Get(),				// texture from previous step
		&srvDesc,								// description from this step
		iblIrradianceCubeMap.GetAddressOf()
	);


	//  STEP 3------------------------------------------------------------------------------------------------
	// Save current render target and depth buffer
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> prevRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> prevDSV;
	context->OMGetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.GetAddressOf());

	// Save curremt viewport
	unsigned int vpCount = 1;
	D3D11_VIEWPORT prevVP = {};
	context->RSGetViewports(&vpCount, &prevVP);


	//  STEP 4------------------------------------------------------------------------------------------------
	// Make sure the viewport matches the texture size
	D3D11_VIEWPORT vp = {};
	vp.Width = (float)iblCubeMapFaceSize;
	vp.Height = (float)iblCubeMapFaceSize;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	context->RSSetViewports(1, &vp);

	// Set states that may or may not be set yet
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//  STEP 5------------------------------------------------------------------------------------------------
	// Set up the shaders used to generate this mao
	AssetManager& assetMngr = AssetManager::GetInstance(); // get the singleton asset manager instance
	assetMngr.GetVertexShader("FullscreenVS.cso")->SetShader();
	SimplePixelShader* irradiancePS = assetMngr.GetPixelShader("IBLIrradianceMapPS.cso");
	irradiancePS->SetShader();
	irradiancePS->SetShaderResourceView("EnvironmentMap", skySRV.Get()); // Skybox texture itself
	irradiancePS->SetSamplerState("BasicSampler", samplerOptions.Get());


	//  STEP 6------------------------------------------------------------------------------------------------
	// Loop through 6 faces of a cube map
	for (int i = 0; i < 6; i++)
	{
		int faceIndex = i; //I like i as the iterator but also want to make this loop a bit more readable
		// Create, clear, and set a new render target view for this face

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv = {};

		// Make a render target view for this face
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;	// this points to a Texture2D array
		rtvDesc.Texture2DArray.ArraySize = 1;			// how much of the array do we need access to?
		rtvDesc.Texture2DArray.FirstArraySlice = faceIndex;	// which texture are we rendering to?
		rtvDesc.Texture2DArray.MipSlice = 0;			// which mip are we rendering into? 
		rtvDesc.Format = textDesc.Format;				// same format as texture
		device->CreateRenderTargetView(irrMapFinalTexture.Get(), &rtvDesc, rtv.GetAddressOf());
		context->OMSetRenderTargets(1, rtv.GetAddressOf(), 0);

		// send data to shaders
		// per-face shader data and copy
		irradiancePS->SetInt("faceIndex", faceIndex);
		irradiancePS->SetFloat("sampleStepPhi", 0.025f);
		irradiancePS->SetFloat("sampleStepTheta", 0.025f);
		irradiancePS->CopyAllBufferData();

		//draw a single triangle and wait for the GPU to finish so we don't stall the drivers
		context->Draw(3, 0); //render exactly 3 vertices
		context->Flush(); // flushing the graphics pipeline so we don't cause a hardware timeout and then possibly a driver crash. (this might make c++ sit and wait for a sec but its better than a crash)


	}
	// Restore the old render target and viewport
	context->OMSetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.Get());
	context->RSSetViewports(1, &prevVP);



}

void Sky::IBLCreateConvolvedSpecularMap()
{
	// Calculate how many mip levels this cube map will need
	// potentially skipping the smallest ones since theyre kind of just the same
	mipLevelCount = max((int)(log2(iblCubeMapFaceSize)) + 1 - mipLevToSkip, 1);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> convSpecMapFinalTex = {};
	//  STEP 1------------------------------------------------------------------------------------------------
	// Create the final convolved specular cube texture
	D3D11_TEXTURE2D_DESC textDesc = {};
	textDesc.Width = iblCubeMapFaceSize;			
	textDesc.Height = iblCubeMapFaceSize;			// want it square
	textDesc.ArraySize = 6;							// a cube has 6 sides
	textDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // will be used as both
	textDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// basic texture format
	textDesc.MipLevels = mipLevelCount;	 // what was calculated before - we actually want mips this time
	textDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // its a cube map
	textDesc.SampleDesc.Count = 1;							//can't be zero
	device->CreateTexture2D(&textDesc, 0, convSpecMapFinalTex.GetAddressOf());

	//  STEP 2------------------------------------------------------------------------------------------------
	// Create SRV for the irradiance texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;	// sample as cube map
	srvDesc.TextureCube.MipLevels = mipLevelCount;
	srvDesc.TextureCube.MostDetailedMip = 0; // the mip in the first position is the most detailed
	srvDesc.Format = textDesc.Format;							// same format as texture
	device->CreateShaderResourceView(
		convSpecMapFinalTex.Get(),				// texture from previous step
		&srvDesc,								// description from this step
		iblConvolvedSpecular.GetAddressOf()
	);


	//  STEP 3------------------------------------------------------------------------------------------------
	// Save current render target and depth buffer
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> prevRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> prevDSV;
	context->OMGetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.GetAddressOf());

	// Save curremt viewport
	unsigned int vpCount = 1;
	D3D11_VIEWPORT prevVP = {};
	context->RSGetViewports(&vpCount, &prevVP);


	//  STEP 4------------------------------------------------------------------------------------------------
	// Set states that may or may not be set yet
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//  STEP 5------------------------------------------------------------------------------------------------
	// Set up the shaders used to generate this mao
	AssetManager& assetMngr = AssetManager::GetInstance(); // get the singleton asset manager instance
	assetMngr.GetVertexShader("FullscreenVS.cso")->SetShader();
	SimplePixelShader* convSpecPS = assetMngr.GetPixelShader("IBLSpecularConvolutionPS.cso");
	convSpecPS->SetShader();
	convSpecPS->SetShaderResourceView("EnvironmentMap", skySRV.Get()); // Skybox texture itself
	convSpecPS->SetSamplerState("BasicSampler", samplerOptions.Get());


	//  STEP 6------------------------------------------------------------------------------------------------
	// Loop through 6 faces of a cube map
	for (int i = 0; i < mipLevelCount; i++)
	{
		int currentMipLevel = i;

		//Make a viewport that matches the size of this MIP level (-1 accounts for 1x1)
		D3D11_VIEWPORT vp = {};
		vp.Width = (float)pow(2, mipLevelCount + mipLevToSkip - 1 - currentMipLevel);
		vp.Height = vp.Width; //want it square
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		context->RSSetViewports(1, &vp);

		for (int i = 0; i < 6; i++)
		{
			int faceIndex = i; //I like i as the iterator but also want to make this loop a bit more readable
			// Create, clear, and set a new render target view for this face

			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv = {};

			// Make a render target view for this face
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;	// this points to a Texture2D array
			rtvDesc.Texture2DArray.ArraySize = 1;			// how much of the array do we need access to?
			rtvDesc.Texture2DArray.FirstArraySlice = faceIndex;	// which texture are we rendering to?
			rtvDesc.Texture2DArray.MipSlice = currentMipLevel;	// which mip are we rendering into? 
			rtvDesc.Format = textDesc.Format;				// same format as texture
			device->CreateRenderTargetView(convSpecMapFinalTex.Get(), &rtvDesc, rtv.GetAddressOf());
			context->OMSetRenderTargets(1, rtv.GetAddressOf(), 0);

			// send data to shaders
			// per-face shader data and copy
			convSpecPS->SetFloat("roughness", currentMipLevel / (float)(mipLevelCount - 1));
			convSpecPS->SetInt("faceIndex", faceIndex);
			convSpecPS->SetInt("mipLevel", currentMipLevel);
			convSpecPS->CopyAllBufferData();

			//draw a single triangle and wait for the GPU to finish so we don't stall the drivers
			context->Draw(3, 0); //render exactly 3 vertices
			context->Flush(); // flushing the graphics pipeline so we don't cause a hardware timeout and then possibly a driver crash. (this might make c++ sit and wait for a sec but its better than a crash)


		}
	}
	// Restore the old render target and viewport
	context->OMSetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.Get());
	context->RSSetViewports(1, &prevVP);


}

void Sky::IBLCreateBRDFLookUpTexture()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> brdfLookupFinalTex = {}; //i think those brackets are right??? im not sure

	//  STEP 1------------------------------------------------------------------------------------------------
	// Create the final irradiance cube texture
	D3D11_TEXTURE2D_DESC textDesc = {};
	textDesc.Width = lookupTexSize;			// make it square
	textDesc.Height = lookupTexSize;			// make it square
	textDesc.ArraySize = 1;							// not a cube map - only one texture instead of 6
	textDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // will be used as both
	textDesc.Format = DXGI_FORMAT_R16G16_UNORM;	// we only need two channels, and we want them to be precise as possible. This one uses double the precision for the r and g channels
	textDesc.MipLevels = 1;							// no mip chain needed
	textDesc.MiscFlags = 0; // not a cube map
	textDesc.SampleDesc.Count = 1;							//can't be zero
	device->CreateTexture2D(&textDesc, 0, brdfLookupFinalTex.GetAddressOf());


	//  STEP 2------------------------------------------------------------------------------------------------
	// Create SRV for the irradiance texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;	// sample as regular 2d texture
	srvDesc.TextureCube.MipLevels = 1;							// only 1 mip level
	srvDesc.TextureCube.MostDetailedMip = 0;					// accessing the only mip
	srvDesc.Format = textDesc.Format;							// same format as texture
	device->CreateShaderResourceView(
		brdfLookupFinalTex.Get(),				// texture from previous step
		&srvDesc,								// description from this step
		iblBrdfLookup.GetAddressOf()
	);


	//  STEP 3------------------------------------------------------------------------------------------------
	// Save current render target and depth buffer
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> prevRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> prevDSV;
	context->OMGetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.GetAddressOf());

	// Save curremt viewport
	unsigned int vpCount = 1;
	D3D11_VIEWPORT prevVP = {};
	context->RSGetViewports(&vpCount, &prevVP);


	//  STEP 4------------------------------------------------------------------------------------------------
	// Make sure the viewport matches the texture size
	D3D11_VIEWPORT vp = {};
	vp.Width = (float)lookupTexSize;
	vp.Height = (float)lookupTexSize;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	context->RSSetViewports(1, &vp);

	// Set states that may or may not be set yet
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//  STEP 5------------------------------------------------------------------------------------------------
	// Set up the shaders used to generate this mao
	AssetManager& assetMngr = AssetManager::GetInstance(); // get the singleton asset manager instance
	assetMngr.GetVertexShader("FullscreenVS.cso")->SetShader();
	SimplePixelShader* brdfLookupPS = assetMngr.GetPixelShader("IBLBrdfLookUpTablePS.cso");
	brdfLookupPS->SetShader();
	brdfLookupPS->SetShaderResourceView("EnvironmentMap", skySRV.Get()); // Skybox texture itself
	brdfLookupPS->SetSamplerState("BasicSampler", samplerOptions.Get());


	//  STEP 6------------------------------------------------------------------------------------------------
	// No loop because we aren't going through a 6 faced cube map - just one texture
	
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv = {};

	// Make a render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;	// this points to a Texture2D (just one, not an array)
	rtvDesc.Texture2DArray.MipSlice = 0;			// which mip are we rendering into? 
	rtvDesc.Format = textDesc.Format;				// same format as texture

	device->CreateRenderTargetView(brdfLookupFinalTex.Get(), &rtvDesc, rtv.GetAddressOf());
	context->OMSetRenderTargets(1, rtv.GetAddressOf(), 0);

	// this shader doesnt need any data sent to it

	//draw a single triangle and wait for the GPU to finish so we don't stall the drivers
	context->Draw(3, 0); //render exactly 3 vertices
	context->Flush(); // flushing the graphics pipeline so we don't cause a hardware timeout and then possibly a driver crash. (this might make c++ sit and wait for a sec but its better than a crash)



	// Restore the old render target and viewport
	context->OMSetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.Get());
	context->RSSetViewports(1, &prevVP);


}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::CreateCubemap(const wchar_t* right, const wchar_t* left, const wchar_t* up, const wchar_t* down, const wchar_t* front, const wchar_t* back)
{
	// Load the 6 textures into an array.
	// - We need references to the TEXTURES, not the SHADER RESOURCE VIEWS!
	// - Specifically NOT generating mipmaps, as we don't need them for the sky!
	// - Order matters here!  +X, -X, +Y, -Y, +Z, -Z
	ID3D11Texture2D* textures[6] = {};
	CreateWICTextureFromFile(device.Get(), right, (ID3D11Resource**)&textures[0], 0);
	CreateWICTextureFromFile(device.Get(), left, (ID3D11Resource**)&textures[1], 0);
	CreateWICTextureFromFile(device.Get(), up, (ID3D11Resource**)&textures[2], 0);
	CreateWICTextureFromFile(device.Get(), down, (ID3D11Resource**)&textures[3], 0);
	CreateWICTextureFromFile(device.Get(), front, (ID3D11Resource**)&textures[4], 0);
	CreateWICTextureFromFile(device.Get(), back, (ID3D11Resource**)&textures[5], 0);

	// We'll assume all of the textures are the same color format and resolution,
	// so get the description of the first shader resource view
	D3D11_TEXTURE2D_DESC faceDesc = {};
	textures[0]->GetDesc(&faceDesc);

	// Describe the resource for the cube map, which is simply 
	// a "texture 2d array".  This is a special GPU resource format, 
	// NOT just a C++ array of textures!!!
	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.ArraySize = 6; // Cube map!
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // We'll be using as a texture in a shader
	cubeDesc.CPUAccessFlags = 0; // No read back
	cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
	cubeDesc.Width = faceDesc.Width;  // Match the size
	cubeDesc.Height = faceDesc.Height; // Match the size
	cubeDesc.MipLevels = 1; // Only need 1
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // This should be treated as a CUBE, not 6 separate textures
	cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;

	// Create the actual texture resource
	ID3D11Texture2D* cubeMapTexture = 0;
	device->CreateTexture2D(&cubeDesc, 0, &cubeMapTexture);

	// Loop through the individual face textures and copy them,
	// one at a time, to the cube map texure
	for (int i = 0; i < 6; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0,	// Which mip (zero, since there's only one)
			i,	// Which array element?
			1); // How many mip levels are in the texture?

		// Copy from one resource (texture) to another
		context->CopySubresourceRegion(
			cubeMapTexture, // Destination resource
			subresource,	// Dest subresource index (one of the array elements)
			0, 0, 0,		// XYZ location of copy
			textures[i],	// Source resource
			0,				// Source subresource index (we're assuming there's only one)
			0);				// Source subresource "box" of data to copy (zero means the whole thing)
	}

	// At this point, all of the faces have been copied into the 
	// cube map texture, so we can describe a shader resource view for it
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format; // Same format as texture
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
	srvDesc.TextureCube.MipLevels = 1;	// Only need access to 1 mip
	srvDesc.TextureCube.MostDetailedMip = 0; // Index of the first mip we want to see

	// Make the SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	device->CreateShaderResourceView(cubeMapTexture, &srvDesc, cubeSRV.GetAddressOf());

	// Now that we're done, clean up the stuff we don't need anymore
	cubeMapTexture->Release();  // Done with this particular reference (the SRV has another)
	for (int i = 0; i < 6; i++)
		textures[i]->Release();

	// Send back the SRV, which is what we need for our shaders
	return cubeSRV;
}
