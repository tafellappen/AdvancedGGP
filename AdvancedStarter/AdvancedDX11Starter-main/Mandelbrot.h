#pragma once
#include <DirectXMath.h>
#include <d3d11.h> // for ID3D11Device and ID3D11DeviceContext
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <vector>

#include "Camera.h"
#include "AssetManager.h"
#include "Material.h"
#include "Input.h"

//----------IDEA DUMP------------
// this could be modified slightly to generate mandelbrot renders of arbitrary size
//		"i have a 16k resolution render of the mandelbrot set" weird flex but ok
// i actually want to be able to save the images generated from this at some point
//-------------------------------

//struct for fractal space zoom - where the screen is in fractal space
struct FractalPlaneInfo 
{
	DirectX::XMFLOAT2 screenMidPosition; //complex plane position of the middle pixel of the screen 
	DirectX::XMFLOAT2 complexMax; //complex plane position of the bottom left of the screen
	DirectX::XMFLOAT2 complexMin; //complex plane position of the top right of the screen
	float scale; 
	//float zoomCount; //yes, a float. Want to be allowed values between integers here for more flexibility

	void MoveScreenPos(float x, float y) {
		screenMidPosition.x += x * scale;//scaling the movement by the same amount as the image itself
		screenMidPosition.y += y * scale;
	};

	void Zoom(float step)
	{
		scale /= step + 1;
	};
};

//class to manage the data used to compute the Mandelbrot set
class Mandelbrot
{
public:
	Mandelbrot(
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		unsigned int windowWidth,
		unsigned int windowHeight,
		Camera* camera,
		Material* material
	);
	
	~Mandelbrot();

	void Update(float dt);
	void Draw(Camera* camera, Sky* sky);
	void PostResize(unsigned int windowWidth, unsigned int windowHeight);
	void RunComputeShader(); //this might only need to run when the position is updated. idk if that would cause a stall but probably not?
	Material* GetMaterial();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV();
private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Camera* camera;
	FractalPlaneInfo fractalPlaneInfo;


	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> computeTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> computeTextureUAV;
	unsigned int windowWidth;
	unsigned int windowHeight;
	DirectX::XMFLOAT2 aspectRatio;
	
	float movementSpeed;
	const float STARTING_SCALE = 1.0;

	SimpleComputeShader* fractalCS;

	Material* material; //i think this was a mistake, its harder to get the control I need. should have looked at Sky from the beginning
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>   mandelbrotRasterState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mandelbrotDepthState;

	//helpers
	void CreateComputeShaderTexture();
	void RetreiveShaders();
	void InitRenderStates();


};
