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
		//movementspeed
		//math is soup i am a fork
		//float diffX = ((complexMax.x - complexMin.x) / 2) / step;
		//complexMin.x = screenMidPosition.x - diffX;
		//complexMax.x = screenMidPosition.x + diffX;
		//float diffY = ((complexMax.y - complexMin.y) / 2) / step;
		//complexMin.y = screenMidPosition.y - diffY;
		//complexMax.y = screenMidPosition.y + diffY;
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
	//const float STARTING_WIDTH_EXTENTS = 2.0; //extents from the origin

	SimpleComputeShader* fractalCS;
	//SimpleVertexShader* fractalVS;
	//SimplePixelShader* fractalPS;

	Material* material; //i think this was a mistake, its harder to get the control I need. should have looked at Sky from the beginning
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>   mandelbrotRasterState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mandelbrotDepthState;


	//Microsoft::WRL::ComPtr<ID3D11Buffer> psPerFrameConstantBuffer;
	//PSPerFrameData psPerFrameData;

	//could use z value for zoom actually
	// 	   maybe save camera starting position, so everything can be relative to that
	// 	   but how would that even work. oh maybe some function of applying the difference in start and current Z value
	// 	   i dont thinK??? that it would fix aforementioned issue
	//     but also this is the simplest way i can think of to do the other things
	// 	   tbh im still not even sure that repurposing the camera like this is a good idea at all
	// 	   but at least i dont have to think about semi-reinventing whatever wheel this would be right now
	// 	   i really hope that doesnt make for a really stupid tradeoff later on
	// 	   but theoretically the way im doing the mapping here should encapsulate this method enough that I dont have to change any other logic here just because i stopped using the camera position
	//plan for now is just take x and z axes of camera. This may backfire if user switches rotation in the other scene though
	void MapWorldToComplex(); //this can almost certainly be removed
	void ResetCamera(); //leave this idea here, it may be useful to have this if something goes wrong. would be funny if this went wrong too though. 

	//helpers
	void FindComplexVisibleExtents();
	float MapValues(float value, float min1, float max1, float min2, float max2);

	void CreateComputeShaderTexture();
	void RetreiveShaders();
	void InitRenderStates();


};

//scale  = cam current z minus cam previous z
//work based on extents rather than arbitrary 
//get center in shader from actual center of the texture