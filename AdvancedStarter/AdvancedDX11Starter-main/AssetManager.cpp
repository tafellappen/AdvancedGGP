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

	//create ClampSampler (for IBL PBR lighting)
	D3D11_SAMPLER_DESC sampClampDesc = {}; //proably unecessary to have this rather than reuse the other but uuuh oh well
	sampClampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampClampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampClampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampClampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampClampDesc.MaxAnisotropy = 16;
	sampClampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampClampDesc, samplerClampOptions.GetAddressOf());

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

	/*	sky = new Sky(
		(assetsFolderPath_Wide + L"\\Skies\\Clouds-Pink\\right.png").c_str(),
		(assetsFolderPath_Wide + L"\\Skies\\Clouds-Pink\\left.png").c_str(),
		(assetsFolderPath_Wide + L"\\Skies\\Clouds-Pink\\up.png").c_str(),
		(assetsFolderPath_Wide + L"\\Skies\\Clouds-Pink\\down.png").c_str(),
		(assetsFolderPath_Wide + L"\\Skies\\Clouds-Pink\\front.png").c_str(),
		(assetsFolderPath_Wide + L"\\Skies\\Clouds-Pink\\back.png").c_str(),
		cubeMesh,
		skyVS,
		skyPS,
		samplerOptions,
		device,
		context
	);
*/


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
				samplerOptions,
				samplerClampOptions
			);
			materials.insert({ matName, thisMat });
			std::cout << "Created material \"" << matName << "\" from definition \"" << fileName << "\"" << std::endl;
		}
		
	}


}

void AssetManager::CreateEntities()
{
	//TODO: want to make this defined by json too

	Mesh* sphereMesh = meshes["sphere.obj"];
	Mesh* helixMesh = meshes["helix.obj"];
	Mesh* cubeMesh = meshes["cube.obj"];
	Mesh* coneMesh = meshes["cone.obj"];


	// === Create the PBR entities =====================================
	GameEntity* cobSpherePBR = new GameEntity(sphereMesh, materials["cobbleMat2xPBR"]);
	cobSpherePBR->GetTransform()->SetScale(2, 2, 2);
	cobSpherePBR->GetTransform()->SetPosition(-6, 2, 0);
	cobSpherePBR->GetMaterial()->SetRefractive(true); //i hate this

	GameEntity* floorSpherePBR = new GameEntity(sphereMesh, materials["floorMatPBR"]);
	floorSpherePBR->GetTransform()->SetScale(2, 2, 2);
	floorSpherePBR->GetTransform()->SetPosition(-4, 2, 0);
	floorSpherePBR->GetMaterial()->SetRefractive(true); //i hate this

	GameEntity* paintSpherePBR = new GameEntity(sphereMesh, materials["paintMatPBR"]);
	paintSpherePBR->GetTransform()->SetScale(2, 2, 2);
	paintSpherePBR->GetTransform()->SetPosition(-2, 2, 0);

	GameEntity* scratchSpherePBR = new GameEntity(sphereMesh, materials["scratchedMatPBR"]);
	scratchSpherePBR->GetTransform()->SetScale(2, 2, 2);
	scratchSpherePBR->GetTransform()->SetPosition(0, 2, 0);

	GameEntity* bronzeSpherePBR = new GameEntity(sphereMesh, materials["bronzeMatPBR"]);
	bronzeSpherePBR->GetTransform()->SetScale(2, 2, 2);
	bronzeSpherePBR->GetTransform()->SetPosition(2, 2, 0);

	GameEntity* roughSpherePBR = new GameEntity(sphereMesh, materials["roughMatPBR"]);
	roughSpherePBR->GetTransform()->SetScale(2, 2, 2);
	roughSpherePBR->GetTransform()->SetPosition(4, 2, 0);

	GameEntity* woodSpherePBR = new GameEntity(sphereMesh, materials["woodMatPBR"]);
	woodSpherePBR->GetTransform()->SetScale(2, 2, 2);
	woodSpherePBR->GetTransform()->SetPosition(6, 2, 0);

	entities.push_back(cobSpherePBR);
	entities.push_back(floorSpherePBR);
	entities.push_back(paintSpherePBR);
	entities.push_back(scratchSpherePBR);
	entities.push_back(bronzeSpherePBR);
	entities.push_back(roughSpherePBR);
	entities.push_back(woodSpherePBR);

	// Create the non-PBR entities ==============================
	GameEntity* cobSphere = new GameEntity(sphereMesh, materials["cobbleMat2x"]);
	cobSphere->GetTransform()->SetScale(2, 2, 2);
	cobSphere->GetTransform()->SetPosition(-6, -2, 0);
	//cobSphere->GetMaterial()->SetRefractive(true); //i hate this so much

	GameEntity* floorSphere = new GameEntity(sphereMesh, materials["floorMat"]);
	floorSphere->GetTransform()->SetScale(2, 2, 2);
	floorSphere->GetTransform()->SetPosition(-4, -2, 0);

	GameEntity* paintSphere = new GameEntity(sphereMesh, materials["paintMat"]);
	paintSphere->GetTransform()->SetScale(2, 2, 2);
	paintSphere->GetTransform()->SetPosition(-2, -2, 0);

	GameEntity* scratchSphere = new GameEntity(sphereMesh, materials["scratchedMat"]);
	scratchSphere->GetTransform()->SetScale(2, 2, 2);
	scratchSphere->GetTransform()->SetPosition(0, -2, 0);

	GameEntity* bronzeSphere = new GameEntity(sphereMesh, materials["bronzeMat"]);
	bronzeSphere->GetTransform()->SetScale(2, 2, 2);
	bronzeSphere->GetTransform()->SetPosition(2, -2, 0);

	GameEntity* roughSphere = new GameEntity(sphereMesh, materials["roughMat"]);
	roughSphere->GetTransform()->SetScale(2, 2, 2);
	roughSphere->GetTransform()->SetPosition(4, -2, 0);

	GameEntity* woodSphere = new GameEntity(sphereMesh, materials["woodMat"]);
	woodSphere->GetTransform()->SetScale(2, 2, 2);
	woodSphere->GetTransform()->SetPosition(6, -2, 0);

	entities.push_back(cobSphere);
	entities.push_back(floorSphere);
	entities.push_back(paintSphere);
	entities.push_back(scratchSphere);
	entities.push_back(bronzeSphere);
	entities.push_back(roughSphere);
	entities.push_back(woodSphere);

	Material* solidMetalMatPBR1 = materials["solidMetalMatPBR1"];
	Material* solidMetalMatPBR2 = materials["solidMetalMatPBR2"];
	Material* solidMetalMatPBR3 = materials["solidMetalMatPBR3"];
	Material* solidMetalMatPBR4 = materials["solidMetalMatPBR4"];
	Material* solidMetalMatPBR5 = materials["solidMetalMatPBR5"];


	GameEntity* solidMetalSphere1 = new GameEntity(sphereMesh, solidMetalMatPBR1);
	GameEntity* solidMetalSphere2 = new GameEntity(sphereMesh, solidMetalMatPBR2);
	GameEntity* solidMetalSphere3 = new GameEntity(sphereMesh, solidMetalMatPBR3);
	GameEntity* solidMetalSphere4 = new GameEntity(sphereMesh, solidMetalMatPBR4);
	GameEntity* solidMetalSphere5 = new GameEntity(sphereMesh, solidMetalMatPBR5);

	solidMetalSphere1->GetTransform()->SetScale(1, 1, 1);
	solidMetalSphere1->GetTransform()->SetPosition(6, 0, 0);

	solidMetalSphere2->GetTransform()->SetScale(1, 1, 1);
	solidMetalSphere2->GetTransform()->SetPosition(4, 0, 0);

	solidMetalSphere3->GetTransform()->SetScale(1, 1, 1);
	solidMetalSphere3->GetTransform()->SetPosition(2, 0, 0);

	solidMetalSphere4->GetTransform()->SetScale(1, 1, 1);
	solidMetalSphere4->GetTransform()->SetPosition(0, 0, 0);

	solidMetalSphere5->GetTransform()->SetScale(1, 1, 1);
	solidMetalSphere5->GetTransform()->SetPosition(-2, 0, 0);

	entities.push_back(solidMetalSphere1);
	entities.push_back(solidMetalSphere2);
	entities.push_back(solidMetalSphere3);
	entities.push_back(solidMetalSphere4);
	entities.push_back(solidMetalSphere5);

	std::cout << "Created entities" << std::endl;
}

void AssetManager::CreateParticleEmitters()
{

	//std::shared_ptr<SimpleVertexShader> vs = std::make_shared<SimpleVertexShader>(assetMngr.GetVertexShader("ParticleVS.cso"));
	//std::shared_ptr<SimplePixelShader> ps = std::make_shared<SimplePixelShader>(assetMngr.GetPixelShader("ParticlePS.cso"));
	SimpleVertexShader* vs = vertexShaders["ParticleVS.cso"];
	SimplePixelShader* ps = pixelShaders["ParticlePS.cso"];

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture1 = textures["dirt_03.png"];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture2 = textures["light_03.png"];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture3 = textures["twirl_03.png"];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture4 = textures["star_07.png"];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture5 = textures["spark_02.png"];

	std::shared_ptr<Emitter> emit1 = std::make_shared<Emitter>(2, 10, 500, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vs, ps, device, context, texture1);
	std::shared_ptr<Emitter> emit2 = std::make_shared<Emitter>(5, 15, 500, XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), vs, ps, device, context, texture2);
	std::shared_ptr<Emitter> emit3 = std::make_shared<Emitter>(6, 18, 500, XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), vs, ps, device, context, texture3);
	std::shared_ptr<Emitter> emit4 = std::make_shared<Emitter>(8, 5, 500, XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), vs, ps, device, context, texture4);
	std::shared_ptr<Emitter> emit5 = std::make_shared<Emitter>(10, 30, 600, XMFLOAT4(0.0f, 1.0f, 0.5f, 1.0f), vs, ps, device, context, texture5);
	//place the emitters
	emit1->GetTransform()->SetPosition(0.0f, 4.0f, 0.0f);
	emit2->GetTransform()->SetPosition(2.0f, 5.0f, 0.0f);
	emit3->GetTransform()->SetPosition(-2.0f, 5.0f, 0.0f);
	emit4->GetTransform()->SetPosition(3.0f, 3.0f, -3.0f);
	emit5->GetTransform()->SetPosition(-3.0f, 3.0f, -3.0f);

	emit2->SetRectBounds(5.0f, 5.0f, 5.0f);
	emit3->SetRectBounds(5.0f, 5.0f, 0.0f);
	emit4->SetRectBounds(5.0f, 0.0f, 5.0f);
	emit5->SetRectBounds(9.0f, 0.0f, 0.0f);

	particleEmitters.push_back(emit1);
	particleEmitters.push_back(emit2);
	particleEmitters.push_back(emit3);
	particleEmitters.push_back(emit4);
	particleEmitters.push_back(emit5);


	std::cout << "Created particle emitters" << std::endl;
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

void AssetManager::LoadComputeShader(std::wstring filepath, std::string shaderName)
{
	SimpleComputeShader* Shader = LoadShader(SimpleComputeShader, filepath);
	computeShaders.insert({ shaderName, Shader });
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AssetManager::GetTexture(std::string name)
{
	return textures[name];
}

Microsoft::WRL::ComPtr<ID3D11SamplerState> AssetManager::GetSamplerOptions()
{
	return samplerOptions;
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

SimpleComputeShader* AssetManager::GetComputeShader(std::string name)
{
	return computeShaders[name];
}

Sky* AssetManager::GetSky()
{
	return sky;
}

std::vector<GameEntity*>* AssetManager::GetEntities()
{
	return &entities;
}

std::vector<std::shared_ptr<Emitter>> AssetManager::GetEmitters()
{
	return particleEmitters;
}
