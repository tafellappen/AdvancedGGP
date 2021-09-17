#include "AssetManager.h"
#include "WICTextureLoader.h"


using namespace DirectX;

// Helper macros for making texture and shader loading code more succinct
#define LoadTexture(file, srv) CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + file).c_str(), 0, srv.GetAddressOf())
#define LoadShader(type, file) new type(device.Get(), context.Get(), filepath.c_str())

// Singleton requirement
AssetManager* AssetManager::instance;


void AssetManager::Initialize(std::string filepath, std::wstring filepathW, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	assetsFolderPath = filepath;
	assetsFolderPath_Wide = filepathW;
	this->device = device;
	this->context = context;
}

AssetManager::~AssetManager()
{

}

void AssetManager::LoadAllAssets()
{
	// Make the meshes
	Mesh* sphereMesh = new Mesh((assetsFolderPath + "/Models/sphere.obj").c_str(), device);
	Mesh* helixMesh =  new Mesh((assetsFolderPath + "/Models/helix.obj").c_str(), device);
	Mesh* cubeMesh =   new Mesh((assetsFolderPath + "/Models/cube.obj").c_str(), device);
	Mesh* coneMesh =   new Mesh((assetsFolderPath + "/Models/cone.obj").c_str(), device);

	meshes.insert({ "sphere", sphereMesh });
	meshes.insert({ "helix", helixMesh });
	meshes.insert({ "cube", cubeMesh });
	meshes.insert({ "cone", coneMesh });

	// Declare the textures we'll need
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleA, cobbleN, cobbleR, cobbleM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorA, floorN, floorR, floorM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintA, paintN, paintR, paintM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedA, scratchedN, scratchedR, scratchedM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeA, bronzeN, bronzeR, bronzeM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughA, roughN, roughR, roughM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodA, woodN, woodR, woodM;

	// Load the textures

	printf((assetsFolderPath + "/Textures/cobblestone_albedo.png").c_str());
	printf("\n");
	/*CreateWICTextureFromFile(
		device.Get(), 
		context.Get(), 
		(assetsFolderPath_Wide + L"/Textures/cobblestone_albedo.png").c_str(), 
		0,
		cobbleA.GetAddressOf()
	);*/
	LoadTexture(L"/Textures/cobblestone_albedo.png", cobbleA);
	LoadTexture(L"/Textures/cobblestone_normals.png", cobbleN);
	LoadTexture(L"/Textures/cobblestone_roughness.png", cobbleR);
	LoadTexture(L"/Textures/cobblestone_metal.png", cobbleM);

	LoadTexture(L"/Textures/floor_albedo.png", floorA);
	LoadTexture(L"/Textures/floor_normals.png", floorN);
	LoadTexture(L"/Textures/floor_roughness.png", floorR);
	LoadTexture(L"/Textures/floor_metal.png", floorM);

	LoadTexture(L"/Textures/paint_albedo.png", paintA);
	LoadTexture(L"/Textures/paint_normals.png", paintN);
	LoadTexture(L"/Textures/paint_roughness.png", paintR);
	LoadTexture(L"/Textures/paint_metal.png", paintM);

	LoadTexture(L"/Textures/scratched_albedo.png", scratchedA);
	LoadTexture(L"/Textures/scratched_normals.png", scratchedN);
	LoadTexture(L"/Textures/scratched_roughness.png", scratchedR);
	LoadTexture(L"/Textures/scratched_metal.png", scratchedM);

	LoadTexture(L"/Textures/bronze_albedo.png", bronzeA);
	LoadTexture(L"/Textures/bronze_normals.png", bronzeN);
	LoadTexture(L"/Textures/bronze_roughness.png", bronzeR);
	LoadTexture(L"/Textures/bronze_metal.png", bronzeM);

	LoadTexture(L"/Textures/rough_albedo.png", roughA);
	LoadTexture(L"/Textures/rough_normals.png", roughN);
	LoadTexture(L"/Textures/rough_roughness.png", roughR);
	LoadTexture(L"/Textures/rough_metal.png", roughM);

	LoadTexture(L"/Textures/wood_albedo.png", woodA);
	LoadTexture(L"/Textures/wood_normals.png", woodN);
	LoadTexture(L"/Textures/wood_roughness.png", woodR);
	LoadTexture(L"/Textures/wood_metal.png", woodM);

	//// Describe and create our sampler state
	//D3D11_SAMPLER_DESC sampDesc = {};
	//sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	//sampDesc.MaxAnisotropy = 16;
	//sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//device->CreateSamplerState(&sampDesc, samplerOptions.GetAddressOf());

}

//it may be word having these also return the pointer to the shader, since it would be nice to have the loading and "shortening" all in one
void AssetManager::LoadVertexShader(std::wstring filepath, std::string shaderName)
{
	SimpleVertexShader* Shader = LoadShader(SimpleVertexShader, filepath);
	vertexShaders.insert({ shaderName, Shader });
}

void AssetManager::LoadPixelShader(std::wstring filepath, std::string shaderName)
{
	SimplePixelShader* Shader = LoadShader(SimplePixelShader, filepath);
	pixelShaders.insert({ shaderName, Shader });
}

Mesh* AssetManager::GetMesh(std::string name)
{
	return meshes[name];
}

Material* AssetManager::GetMaterial(std::string name)
{
	return nullptr;
}

SimpleVertexShader* AssetManager::GetVertexShader(std::string name)
{
	return vertexShaders[name];
}

SimplePixelShader* AssetManager::GetPixelShader(std::string name)
{
	return pixelShaders[name];
}
