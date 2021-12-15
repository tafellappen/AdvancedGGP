#pragma once
#include <DirectXMath.h>
#include <d3d11.h> // for ID3D11Device and ID3D11DeviceContext
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <vector>

#include "Camera.h"
#include "AssetManager.h"

//----------IDEA DUMP------------
// this could be modified slightly to generate mandelbrot renders of arbitrary size
//		"i have a 16k resolution render of the mandelbrot set" weird flex but ok
// i actually want to be able to save the images generated from this at some point
// left off at about 50 minutes in lecture
//-------------------------------


//class to manage the data used to compute the Mandelbrot set
class Mandelbrot
{
public:
	Mandelbrot(
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		unsigned int windowWidth,
		unsigned int windowHeight,
		Camera* camera
	);

	void Update();
	void PostResize(unsigned int windowWidth, unsigned int windowHeight);
	void RunComputeShader(); //this might only need to run when the position is updated. idk if that would cause a stall but probably not?
private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Camera* camera;

	DirectX::XMFLOAT2 complexPos;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> computeTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> computeTextureUAV;
	unsigned int windowWidth;
	unsigned int windowHeight;

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
	void MapCamWorldToComplex(); 
	void ResetCamera(); //leave this idea here, it may be useful to have this if something goes wrong. would be funny if this went wrong too though. 

	//helpers
	void CreateComputeShaderTexture();
};

