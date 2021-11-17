#include "Renderer.h"

#include "imgui/imgui.h"

#include "imgui/imgui_impl_dx11.h" //visual studio wanted me to have these too, why did just the first one not work?
#include "imgui/imgui_impl_win32.h"


#include "Input.h"//not sure if this is safe here but i need this for the HandleGuiUpdate method. probably a safer way to do this with a using statement or whatever cpp equivalent

#include "AssetManager.h"

using namespace DirectX;


Renderer::Renderer(
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

):
	vsPerFrameConstantBuffer(0),
	psPerFrameConstantBuffer(0),
	ambientNonPBR(0.1f, 0.1f, 0.25f),
	refractionScale(0.1f),
	refractionFromNormalMap(true),
	indexOfRefraction(0.5f)
{
	this->device = device;
	this->context = context;
	this->swapChain = swapChain;
	this->backBufferRTV = backBufferRTV;
	this->depthBufferDSV = depthBufferDSV;
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	this->sky = sky;
	this->entities = entities;
	this->lights = lights;

	this->lightMesh = lightMesh;
	this->lightVS = lightVS;
	this->lightPS = lightPS;
	//this->lightCount = lightCount;

    // Initialize structs
	vsPerFrameData = {};
	psPerFrameData = {};

	// Grab two shaders on which to base per-frame cbuffers
	// Note: We're assuming ALL entity/material per-frame buffers are identical!
	//       And that they're all called "perFrame"
	AssetManager& assets = AssetManager::GetInstance();
	SimplePixelShader* ps = assets.GetPixelShader("PixelShaderPBR.cso");
	SimpleVertexShader* vs = assets.GetVertexShader("VertexShader.cso");

	// Struct to hold the descriptions from existing buffers
	D3D11_BUFFER_DESC bufferDesc = {};
	const SimpleConstantBuffer* scb = 0;

	// Make a new buffer that matches the existing PS per-frame buffer
	scb = ps->GetBufferInfo("perFrame");
	scb->ConstantBuffer.Get()->GetDesc(&bufferDesc);
	device->CreateBuffer(&bufferDesc, 0, psPerFrameConstantBuffer.GetAddressOf());

	// Make a new buffer that matches the existing PS per-frame buffer
	scb = vs->GetBufferInfo("perFrame");
	scb->ConstantBuffer.Get()->GetDesc(&bufferDesc);
	device->CreateBuffer(&bufferDesc, 0, vsPerFrameConstantBuffer.GetAddressOf());

	PostResize(windowWidth, windowHeight, backBufferRTV, depthBufferDSV);

	// Depth state for refraction silhouette
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = true;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // No depth writing
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&depthDesc, refractionSilhouetteDepthState.GetAddressOf());

}

Renderer::~Renderer()
{
	//These are a reference that should be cleaned up in game, not here
	//for (auto& e : entities) delete e;
	//delete entities;
//	entities = nullptr;
//	lights = nullptr;
//	//for (auto& l : *lights) delete l;
//}
	//delete sky;
	sky = nullptr;

	lightMesh = nullptr;
	lightVS = nullptr;
	lightPS = nullptr;


}

void Renderer::PostResize(
	unsigned int windowWidth, 
	unsigned int windowHeight, 
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferRTV, 
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthBufferDSV)
{
	this->backBufferRTV = backBufferRTV;
	this->depthBufferDSV = depthBufferDSV;
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;

	//render target views
	sceneColorsRTV.Reset();
	sceneNormalsRTV.Reset();
	sceneDepthsRTV.Reset();
	refractionSilhouetteRTV.Reset();
	finalCompositeRTV.Reset();
	//shader resource views
	sceneColorsSRV.Reset();
	sceneNormalsSRV.Reset();
	sceneDepthsSRV.Reset();
	refractionSilhouetteSRV.Reset();
	finalCompositeSRV.Reset();

	CreateRenderTarget(windowWidth, windowHeight, sceneColorsRTV, sceneColorsSRV);
	CreateRenderTarget(windowWidth, windowHeight, sceneNormalsRTV, sceneNormalsSRV);
	CreateRenderTarget(windowWidth, windowHeight, sceneDepthsRTV, sceneDepthsSRV);
	CreateRenderTarget(windowWidth, windowHeight, refractionSilhouetteRTV, refractionSilhouetteSRV);
	CreateRenderTarget(windowWidth, windowHeight, finalCompositeRTV, finalCompositeSRV);


}

void Renderer::UpdateLightVec(std::vector<Light> lights)
{
	this->lights = lights;
}


void Renderer::Render(Camera* camera, int lightCount)
{
	// Background color for clearing
	const float color[4] = { 0, 0, 0, 1 };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,	0);

	//clear refraction render targets
	context->ClearRenderTargetView(sceneColorsRTV.Get(), color);
	context->ClearRenderTargetView(sceneNormalsRTV.Get(), color);
	context->ClearRenderTargetView(refractionSilhouetteRTV.Get(), color);
	context->ClearRenderTargetView(finalCompositeRTV.Get(), color);
	const float depth[4] = { 1,0,0,0 };
	context->ClearRenderTargetView(sceneDepthsRTV.Get(), depth);

	const int numTargets = 3;
	ID3D11RenderTargetView* targets[numTargets] = {};
	targets[0] = sceneColorsRTV.Get();
	targets[1] = sceneNormalsRTV.Get();
	targets[2] = sceneDepthsRTV.Get();
	//targets[3] = refractionSilhouetteRTV.Get(); //no i dont think i need you here
	context->OMSetRenderTargets(numTargets, targets, depthBufferDSV.Get());

	// Collect all per-frame data and copy to GPU
	{
		// vs ----
		vsPerFrameData.ViewMatrix = camera->GetView();
		vsPerFrameData.ProjectionMatrix = camera->GetProjection();
		context->UpdateSubresource(vsPerFrameConstantBuffer.Get(), 0, 0, &vsPerFrameData, 0, 0);

		// ps ----
		memcpy(&psPerFrameData.Lights, &lights[0], sizeof(Light) * lightCount);
		psPerFrameData.LightCount = lightCount;
		psPerFrameData.CameraPosition = camera->GetTransform()->GetPosition();
		psPerFrameData.TotalSpecIBLMipLevels = sky->GetMipLevelCount();
		psPerFrameData.AmbientNonPBR = ambientNonPBR;
		context->UpdateSubresource(psPerFrameConstantBuffer.Get(), 0, 0, &psPerFrameData, 0, 0);
	}

	std::vector<GameEntity*> refractiveEntities;
	// Draw all of the non-refractive entities
	for (auto ge : entities)
	{
		// Skip refractive materials for now
		if (ge->GetMaterial()->GetRefractive())
		{
			refractiveEntities.push_back(ge);
			//continue;
		}
		else
		{

			// Set the "per frame" data 
			// Note that this should literally be set once PER FRAME, before 
			// the draw loop, but we're currently setting it per entity since  
			// we are just using whichever shader the current entity has.   
			// Inefficient!!! 
			SimplePixelShader* ps = ge->GetMaterial()->GetPS();
			ps->SetData("Lights", (void*)(&lights[0]), sizeof(Light) * lightCount);
			ps->SetInt("LightCount", lightCount);
			ps->SetFloat3("CameraPosition", camera->GetTransform()->GetPosition());
			ps->SetInt("SpecIBLTotalMipLevels", sky->GetMipLevelCount());
			ps->CopyBufferData("perFrame");

			// Draw the entity 
			ge->Draw(context, camera, sky);

		}

	}

	//-----------END ALL SOLID/OPAQUE OBJECTS----------------------------------------------
	// Draw the sky, AFTER all opaque but BEFORE transparent ones
	sky->Draw(camera);

	// Get data asset manager
	AssetManager& assets = AssetManager::GetInstance();
	SimpleVertexShader* vs = assets.GetVertexShader("FullscreenVS.cso");
	vs->SetShader();

	// Final combine
	{
		// Set up final combine
		targets[0] = finalCompositeRTV.Get();
		context->OMSetRenderTargets(1, targets, 0);

		SimplePixelShader* ps = assets.GetPixelShader("CombinePS.cso");
		ps->SetShader();
		ps->SetShaderResourceView("SceneColors", sceneColorsSRV);
		ps->CopyAllBufferData();
		context->Draw(3, 0);
	}

	// Draw the solid objects to the screen
	{
		targets[0] = backBufferRTV.Get();
		context->OMSetRenderTargets(1, targets, 0);
		SimplePixelShader* ps = assets.GetPixelShader("SimpleTexturePS.cso");
		ps->SetShader();
		ps->SetShaderResourceView("Pixels", finalCompositeSRV.Get());
		context->Draw(3, 0);
	}

	//---Refraction---
	{
		//making the refraction silhouette
		{
			//loop and render the refractive objects to the texture for the silhouette, as a solid color

			targets[0] = refractionSilhouetteRTV.Get();
			context->OMSetRenderTargets(1, targets, depthBufferDSV.Get());

			// Depth state
			context->OMSetDepthStencilState(refractionSilhouetteDepthState.Get(), 0);

			// Grab the solid color shader
			SimplePixelShader* solidColorPS = assets.GetPixelShader("SolidColorPS.cso");


			for (auto ge : refractiveEntities)
			{
				// Get this material and sub the refraction PS for now
				Material* mat = ge->GetMaterial();
				SimplePixelShader* prevPS = mat->GetPS();
				mat->SetPS(solidColorPS);

				// Overall material prep
				mat->PrepareMaterial(ge->GetTransform(), camera, sky);
				mat->SetPerMaterialDataAndResources(true, sky);

				// Set up the refraction specific data
				solidColorPS->SetFloat3("Color", XMFLOAT3(1, 1, 1));
				solidColorPS->CopyBufferData("externalData");

				// Reset "per frame" buffer for VS
				context->VSSetConstantBuffers(0, 1, vsPerFrameConstantBuffer.GetAddressOf());

				// Draw
				ge->GetMesh()->SetBuffersAndDraw(context);

				// Reset this material's PS
				mat->SetPS(prevPS);
			}

			// Reset depth state
			context->OMSetDepthStencilState(0, 0);
		}
		// Loop and draw refractive objects
		{
			// Set up pipeline for refractive draw
			// Same target (back buffer), but now we need the depth buffer again
			targets[0] = backBufferRTV.Get();
			context->OMSetRenderTargets(1, targets, depthBufferDSV.Get());

			// Grab the refractive shader
			SimplePixelShader* refractionPS = assets.GetPixelShader("RefractionPS.cso");

			// Loop and draw each one
			for (auto ge : refractiveEntities)
			{
				// Get this material and sub the refraction PS for now
				Material* mat = ge->GetMaterial();
				SimplePixelShader* prevPS = mat->GetPS();
				mat->SetPS(refractionPS);

				// Overall material prep
				mat->PrepareMaterial(ge->GetTransform(), camera, sky);
				mat->SetPerMaterialDataAndResources(true, sky);

				// Set up the refraction specific data
				refractionPS->SetFloat2("screenSize", XMFLOAT2((float)windowWidth, (float)windowHeight));
				refractionPS->SetMatrix4x4("viewMatrix", camera->GetView());
				refractionPS->SetMatrix4x4("projMatrix", camera->GetProjection());
				//refractionPS->SetInt("useRefractionSilhouette", useRefractionSilhouette);
				refractionPS->SetInt("refractionFromNormalMap", refractionFromNormalMap);
				refractionPS->SetFloat("indexOfRefraction", indexOfRefraction);
				refractionPS->SetFloat("refractionScale", refractionScale);
				refractionPS->CopyBufferData("perObject");

				// Set textures
				refractionPS->SetShaderResourceView("ScreenPixels", finalCompositeSRV.Get());
				refractionPS->SetShaderResourceView("RefractionSilhouette", refractionSilhouetteSRV.Get());
				refractionPS->SetShaderResourceView("EnvironmentMap", sky->GetEnvironmentMap());


				// Reset "per frame" buffers
				context->VSSetConstantBuffers(0, 1, vsPerFrameConstantBuffer.GetAddressOf());
				context->PSSetConstantBuffers(0, 1, psPerFrameConstantBuffer.GetAddressOf());

				// Draw
				ge->GetMesh()->SetBuffersAndDraw(context);

				// Reset this material's PS
				mat->SetPS(prevPS);
			}
		}
	}

	// Draw the light sources
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	DrawPointLights(camera, lightCount);



	// Draw some UI
	//DrawUI();

	//draw ImGUI
	ImGui::Render();;
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());

	// Unbind all SRVs at the end of the frame so they're not still bound for input
	// when we begin the MRTs of the next frame
	ID3D11ShaderResourceView* nullSRVs[16] = {};
	context->PSSetShaderResources(0, 16, nullSRVs);
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetSceneColorsSRV()
{
	return sceneColorsSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetSceneNormalsSRV()
{
	return sceneNormalsSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetSceneDepthsSRV()
{
	return sceneDepthsSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetRefractionSilhouetteSRV()
{
	return refractionSilhouetteSRV;
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Renderer::GetFinalCompositeSRV()
{
	return finalCompositeSRV;
}

void Renderer::DrawPointLights(Camera* camera, int lightCount)
{
	AssetManager& assetMngr = AssetManager::GetInstance();


	// Turn on these shaders
	lightVS->SetShader();
	lightPS->SetShader();

	// Set up vertex shader
	lightVS->SetMatrix4x4("view", camera->GetView());
	lightVS->SetMatrix4x4("projection", camera->GetProjection());

	for (int i = 0; i < lightCount; i++)
	{
		Light light = lights[i];

		// Only drawing points, so skip others
		if (light.Type != LIGHT_TYPE_POINT)
			continue;

		// Calc quick scale based on range
		// (assuming range is between 5 - 10)
		float scale = light.Range / 10.0f;

		// Make the transform for this light
		XMMATRIX rotMat = XMMatrixIdentity();
		XMMATRIX scaleMat = XMMatrixScaling(scale, scale, scale);
		XMMATRIX transMat = XMMatrixTranslation(light.Position.x, light.Position.y, light.Position.z);
		XMMATRIX worldMat = scaleMat * rotMat * transMat;

		XMFLOAT4X4 world;
		XMFLOAT4X4 worldInvTrans;
		XMStoreFloat4x4(&world, worldMat);
		XMStoreFloat4x4(&worldInvTrans, XMMatrixInverse(0, XMMatrixTranspose(worldMat)));

		// Set up the world matrix for this light
		lightVS->SetMatrix4x4("world", world);
		lightVS->SetMatrix4x4("worldInverseTranspose", worldInvTrans);

		// Set up the pixel shader data
		XMFLOAT3 finalColor = light.Color;
		finalColor.x *= light.Intensity;
		finalColor.y *= light.Intensity;
		finalColor.z *= light.Intensity;
		lightPS->SetFloat3("Color", finalColor);

		// Copy data
		lightVS->CopyAllBufferData();
		lightPS->CopyAllBufferData();

		// Draw
		lightMesh->SetBuffersAndDraw(context);
	}

}

void Renderer::CreateRenderTarget(unsigned int width, unsigned int height, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv, DXGI_FORMAT colorFormat)
{
	// Make the texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> rtTexture;

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // Need both!
	texDesc.Format = colorFormat;
	texDesc.MipLevels = 1; // Usually no mip chain needed for render targets
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1; // Can't be zero
	device->CreateTexture2D(&texDesc, 0, rtTexture.GetAddressOf());

	// Make the render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // This points to a Texture2D
	rtvDesc.Texture2D.MipSlice = 0;                             // Which mip are we rendering into?
	rtvDesc.Format = texDesc.Format;                // Same format as texture
	device->CreateRenderTargetView(rtTexture.Get(), &rtvDesc, rtv.GetAddressOf());

	// Create the shader resource view using default options 
	device->CreateShaderResourceView(
		rtTexture.Get(),     // Texture resource itself
		0,                   // Null description = default SRV options
		srv.GetAddressOf()); // ComPtr<ID3D11ShaderResourceView>
}


