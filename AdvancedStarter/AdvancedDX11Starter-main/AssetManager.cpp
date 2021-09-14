#include "AssetManager.h"
#include "WICTextureLoader.h"


using namespace DirectX;

// Helper macros for making texture and shader loading code more succinct
#define LoadTexture(file, srv) CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + "/Textures/cobblestone_albedo.png").c_str(), 0, srv.GetAddressOf())
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

	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/cobblestone_albedo.png").c_str(), cobbleA.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/cobblestone_normals.png").c_str(), cobbleN.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/cobblestone_roughness.png").c_str(), cobbleR.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/cobblestone_metal.png").c_str(), cobbleM.GetAddressOf());

	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/floor_albedo.png").c_str(), floorA.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/floor_normals.png").c_str(), floorN.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/floor_roughness.png").c_str(), floorR.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/floor_metal.png").c_str(), floorM.GetAddressOf());

	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/paint_albedo.png").c_str(), paintA.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/paint_normals.png").c_str(), paintN.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/paint_roughness.png").c_str(), paintR.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/paint_metal.png").c_str(), paintM.GetAddressOf());

	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/scratched_albedo.png").c_str(), scratchedA.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/scratched_normals.png").c_str(), scratchedN.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/scratched_roughness.png").c_str(), scratchedR.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/scratched_metal.png").c_str(), scratchedM.GetAddressOf());

	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/bronze_albedo.png").c_str(), bronzeA.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/bronze_normals.png").c_str(), bronzeN.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/bronze_roughness.png").c_str(), bronzeR.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/bronze_metal.png").c_str(), bronzeM.GetAddressOf());

	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/rough_albedo.png").c_str(), roughA.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/rough_normals.png").c_str(), roughN.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/rough_roughness.png").c_str(), roughR.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/rough_metal.png").c_str(), roughM.GetAddressOf());

	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/wood_albedo.png").c_str(), woodA.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/wood_normals.png").c_str(), woodN.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/wood_roughness.png").c_str(), woodR.GetAddressOf());
	//CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + L"/Textures/wood_metal.png").c_str(), woodM.GetAddressOf());


}

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
	return nullptr;
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
