#include "AssetManager.h"
#include "WICTextureLoader.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp> //nuget package, for parsing json definitions of things like materials


//#include <filesystem>
using namespace DirectX;
using json = nlohmann::json;
//namespace fs = std::filesystem;

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
	for (auto& meshItem : meshes)
	{
		delete meshItem.second;
		meshItem.second = nullptr;
	}

	for (auto& matItem : materials)
	{
		delete matItem.second;
		matItem.second = nullptr;
	}

	for (auto& shadItem : vertexShaders)
	{
		delete shadItem.second;
		shadItem.second = nullptr;
	}

	for (auto& shadItem : pixelShaders)
	{
		delete shadItem.second;
		shadItem.second = nullptr;
	}
	////meshes.clear();
	////materials.clear();
	////vertexShaders.clear();
	////pixelShaders.clear();
	delete sky;
	sky = nullptr;
}

void AssetManager::LoadAllAssets()
{
	// https://docs.w3cub.com/cpp/filesystem/recursive_directory_iterator
	for (auto& p : std::filesystem::recursive_directory_iterator(assetsFolderPath))
	{
		std::string filepath = p.path().string();
		std::wstring filepath_wide = p.path().wstring(); //need wstring for CreateWICTextureFromFile
		std::string relativeFilepath = p.path().relative_path().string();
		std::string fileName = p.path().filename().string();
		std::string fileExtension = p.path().extension().string();

		if (fileExtension == ".obj")
		{
			meshes.insert({
				fileName,
				new Mesh(filepath.c_str(), device)
				});
			std::cout << "Loaded mesh " << fileName << std::endl;
		}
		else if (fileExtension == ".png")
		{
			//need to declare  the ShaderResourceView first
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tex;

			CreateWICTextureFromFile(
				device.Get(),
				context.Get(),
				(filepath_wide).c_str(),
				0,
				tex.GetAddressOf()
			);

			textures.insert({
				fileName,
				tex
				});

			std::cout << "Loaded texture " << fileName << std::endl;
		}



	}



	// Describe and create our sampler state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDesc, samplerOptions.GetAddressOf()); //todo: im pretty sure sampler state can just stay in this class and doesnt even need to be accessed in game
	// emphasis on "pretty sure"

	//for now assume that these already exist, aka assume that all shaders were loaded before this.
	//todo: I'd really like to be able to put these both in their own method, and have this just automatically handle whether its a dds cube map or 6 images
	SimpleVertexShader* skyVS = vertexShaders["SkyVS.cso"];
	SimplePixelShader* skyPS = pixelShaders["SkyPS.cso"];
	Mesh* cubeMesh = meshes["cube.obj"];
	// Create the sky using a DDS cube map
/*sky = new Sky(
	GetFullPathTo_Wide(L"..\\..\\Assets\\Skies\\SunnyCubeMap.dds").c_str(),
	cubeMesh,
	skyVS,
	skyPS,
	samplerOptions,
	device,
	context);*/

	sky = new Sky(
		(assetsFolderPath_Wide + L"\\Skies\\Night\\right.png").c_str(),
		(assetsFolderPath_Wide + L"\\Skies\\Night\\left.png").c_str(),
		(assetsFolderPath_Wide + L"\\Skies\\Night\\up.png").c_str(),
		(assetsFolderPath_Wide + L"\\Skies\\Night\\down.png").c_str(),
		(assetsFolderPath_Wide + L"\\Skies\\Night\\front.png").c_str(),
		(assetsFolderPath_Wide + L"\\Skies\\Night\\back.png").c_str(),
		cubeMesh,
		skyVS,
		skyPS,
		samplerOptions,
		device,
		context
	);



	//go through all the material definitions
	for (auto& p : std::filesystem::recursive_directory_iterator(assetsFolderPath + "/ObjectDefinitions/Materials"))
	{
		std::string fileName = p.path().filename().string();
		std::string filepath = p.path().string();
		std::string fileExtension = p.path().extension().string();

		if (fileExtension == ".json")
		{
			//std::cout << "Found material json file: " << filepath << std::endl;
			std::ifstream matDefFile = std::ifstream(filepath);
			//https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
			std::string matDefStr((std::istreambuf_iterator<char>(matDefFile)), (std::istreambuf_iterator<char>()));
			json materialDefinition = json::parse(matDefStr);

			std::string matName = materialDefinition["name"];
			std::vector<float> matColor = materialDefinition["color"];

			std::string texAlbedo = materialDefinition["textures"]["albedo"];//.value("albedo", undefinedDefault);
			std::string texNorm = materialDefinition["textures"]["normal"];//.value("albedo", undefinedDefault);
			std::string texRough = materialDefinition["textures"]["rough"];//.value("albedo", undefinedDefault);
			std::string texMetal = materialDefinition["textures"]["metal"];//.value("albedo", undefinedDefault);
			
			std::vector<float> matUvScale = materialDefinition["uvScale"];

			SimpleVertexShader* vertexShader = vertexShaders[materialDefinition["vertexShader"]];
			SimplePixelShader* pixelShader = pixelShaders[materialDefinition["pixelShader"]];

			Material* thisMat = new Material(
				vertexShader,
				pixelShader,
				XMFLOAT4(matColor[0], matColor[1], matColor[2], matColor[3]),
				256.0f,
				XMFLOAT2(matUvScale[0], matUvScale[1]),
				textures[texAlbedo],
				textures[texNorm],
				textures[texRough],
				textures[texMetal],
				samplerOptions
			);
			materials.insert({ matName, thisMat });
			std::cout << "Created material \"" << matName << "\" from definition \"" << fileName << "\"" << std::endl;
		}
		
	}


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
	return materials[name];
}

SimpleVertexShader* AssetManager::GetVertexShader(std::string name)
{
	return vertexShaders[name];
}

SimplePixelShader* AssetManager::GetPixelShader(std::string name)
{
	return pixelShaders[name];
}

Sky* AssetManager::GetSky()
{
	return sky;
}
