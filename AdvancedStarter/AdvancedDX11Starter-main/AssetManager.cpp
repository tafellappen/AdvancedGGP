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

}

void AssetManager::LoadAllAssets()
{
	/*std::experimental::filesystem::recursive_directory_iterator fileIterator = std::experimental::filesystem::recursive_directory_iterator();
	for (std::experimental::filesystem::recursive_directory_iterator next((filepath)), end; next != end; ++next)
	{

	}*/
	// https://docs.w3cub.com/cpp/filesystem/recursive_directory_iterator
	for (auto& p : std::filesystem::recursive_directory_iterator(assetsFolderPath))
	{
		//std::cout << p.path() << '\n';
		//std::cout << p.path().filename().string() << '\n';
		//std::cout << p.path().extension().string() << '\n';

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
			std::cout << "Loaded mesh " << relativeFilepath << '\n';
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

			std::cout << "Loaded texture " << relativeFilepath << '\n';
		}



	}

	//// Make the meshes
	//Mesh* sphereMesh = new Mesh((assetsFolderPath + "/Models/sphere.obj").c_str(), device);
	//Mesh* helixMesh =  new Mesh((assetsFolderPath + "/Models/helix.obj").c_str(), device);
	//Mesh* cubeMesh =   new Mesh((assetsFolderPath + "/Models/cube.obj").c_str(), device);
	//Mesh* coneMesh =   new Mesh((assetsFolderPath + "/Models/cone.obj").c_str(), device);

	//meshes.insert({ "sphere", sphereMesh });
	//meshes.insert({ "helix", helixMesh });
	//meshes.insert({ "cube", cubeMesh });
	//meshes.insert({ "cone", coneMesh });

	// Declare the textures we'll need
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleA, cobbleN, cobbleR, cobbleM;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorA, floorN, floorR, floorM;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintA, paintN, paintR, paintM;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedA, scratchedN, scratchedR, scratchedM;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeA, bronzeN, bronzeR, bronzeM;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughA, roughN, roughR, roughM;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodA, woodN, woodR, woodM;

	//// Load the textures
	//LoadTexture(L"/Textures/cobblestone_albedo.png", cobbleA);
	//LoadTexture(L"/Textures/cobblestone_normals.png", cobbleN);
	//LoadTexture(L"/Textures/cobblestone_roughness.png", cobbleR);
	//LoadTexture(L"/Textures/cobblestone_metal.png", cobbleM);

	//LoadTexture(L"/Textures/floor_albedo.png", floorA);
	//LoadTexture(L"/Textures/floor_normals.png", floorN);
	//LoadTexture(L"/Textures/floor_roughness.png", floorR);
	//LoadTexture(L"/Textures/floor_metal.png", floorM);

	//LoadTexture(L"/Textures/paint_albedo.png", paintA);
	//LoadTexture(L"/Textures/paint_normals.png", paintN);
	//LoadTexture(L"/Textures/paint_roughness.png", paintR);
	//LoadTexture(L"/Textures/paint_metal.png", paintM);

	//LoadTexture(L"/Textures/scratched_albedo.png", scratchedA);
	//LoadTexture(L"/Textures/scratched_normals.png", scratchedN);
	//LoadTexture(L"/Textures/scratched_roughness.png", scratchedR);
	//LoadTexture(L"/Textures/scratched_metal.png", scratchedM);

	//LoadTexture(L"/Textures/bronze_albedo.png", bronzeA);
	//LoadTexture(L"/Textures/bronze_normals.png", bronzeN);
	//LoadTexture(L"/Textures/bronze_roughness.png", bronzeR);
	//LoadTexture(L"/Textures/bronze_metal.png", bronzeM);

	//LoadTexture(L"/Textures/rough_albedo.png", roughA);
	//LoadTexture(L"/Textures/rough_normals.png", roughN);
	//LoadTexture(L"/Textures/rough_roughness.png", roughR);
	//LoadTexture(L"/Textures/rough_metal.png", roughM);

	//LoadTexture(L"/Textures/wood_albedo.png", woodA);
	//LoadTexture(L"/Textures/wood_normals.png", woodN);
	//LoadTexture(L"/Textures/wood_roughness.png", woodR);
	//LoadTexture(L"/Textures/wood_metal.png", woodM);

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


	// Create basic materials
	//Material* cobbleMat2x = new Material(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), cobbleA, cobbleN, cobbleR, cobbleM, samplerOptions);
	//std::string matDefStr = "";
	//if (matDefFile.is_open())
	//{
	//	while (matDefFile.good())
	//	{
	//		matDefStr += matDefFile.getline()
	//	}
	//	matDefFile.close();
	//}

	//go through all the material definitions
	for (auto& p : std::filesystem::recursive_directory_iterator(assetsFolderPath + "/ObjectDefinitions/Materials"))
	{
		//std::wstring filepath_wide = p.path().wstring(); //need wstring for CreateWICTextureFromFile
		//std::string relativeFilepath = p.path().relative_path().string();
		//std::string fileName = p.path().filename().string();
		std::string filepath = p.path().string();
		std::string fileExtension = p.path().extension().string();

		if (fileExtension == ".json")
		{
			std::cout << "Found material json file: " << filepath << std::endl;
			std::ifstream matDefFile = std::ifstream(filepath);
			//https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
			std::string matDefStr((std::istreambuf_iterator<char>(matDefFile)), (std::istreambuf_iterator<char>()));
			//std::cout << "json file: " << std::endl;
			////std::cout << matDefFile << std::endl;
			json materialDefinition = json::parse(matDefStr);// nullptr, false);
			//std::cout << materialDefinition << std::endl;

			std::string matName = materialDefinition["name"];
			std::vector<float> matColor = materialDefinition["color"];
			//XMFLOAT4 matColor = XMFLOAT4(materialDefinition["color"]);

			std::string texAlbedo = materialDefinition["textures"]["albedo"];//.value("albedo", undefinedDefault);
			std::string texNorm = materialDefinition["textures"]["normal"];//.value("albedo", undefinedDefault);
			std::string texRough = materialDefinition["textures"]["rough"];//.value("albedo", undefinedDefault);
			std::string texMetal = materialDefinition["textures"]["metal"];//.value("albedo", undefinedDefault);
			
			std::vector<float> matUvScale = materialDefinition["uvScale"];

			SimpleVertexShader* vertexShader = vertexShaders[materialDefinition["vertexShader"]];
			SimplePixelShader* pixelShader = pixelShaders[materialDefinition["pixelShader"]];

			/*json textureDefObj = json::parse(texturesStr);
			std::string texAlbedo = textureDefObj.value("albedo", undefinedDefault);*/
			std::cout << texAlbedo << std::endl;
			std::cout << texNorm << std::endl;
			std::cout << texRough << std::endl;
			std::cout << texMetal << std::endl;

			Material* thisMat = new Material(
				vertexShader,
				pixelShader,
				XMFLOAT4(1, 1, 1, 1),
				256.0f,
				XMFLOAT2(2, 2),
				textures[texAlbedo],
				textures[texNorm],
				textures[texRough],
				textures[texMetal],
				samplerOptions
			);
			materials.insert({ matName, thisMat });
			std::cout << "Created material: " << matName << std::endl;
		}
		
	}

	//std::ifstream matDefFile = std::ifstream(assetsFolderPath + "/ObjectDefinitions/Materials/cobbleMat2x.json");
	////https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
	//std::string matDefStr((std::istreambuf_iterator<char>(matDefFile)), (std::istreambuf_iterator<char>()));
	////std::cout << "json file: " << std::endl;
	//////std::cout << matDefFile << std::endl;
	//json materialDefinition = json::parse(matDefStr);// nullptr, false);
	////std::cout << materialDefinition << std::endl;


	//std::string undefinedDefault = "undefined"; //default value for if the value is not found in the json

	//SimpleVertexShader* vertexShader = vertexShaders[materialDefinition["vertexShader"]];
	//SimplePixelShader* pixelShader = pixelShaders[materialDefinition["pixelShader"]];
	////SimpleVertexShader* vertexShader = vertexShaders[materialDefinition.value("vertexShader", undefinedDefault)];
	////SimplePixelShader* pixelShader = pixelShaders[materialDefinition.value("pixelShader", undefinedDefault)];
	//SimplePixelShader* pixelShaderPBR = pixelShaders["PixelShaderPBR.cso"];
	//std::string texAlbedo = materialDefinition["textures"]["albedo"];//.value("albedo", undefinedDefault);
	//std::string texNorm = materialDefinition["textures"]["normal"];//.value("albedo", undefinedDefault);
	//std::string texRough = materialDefinition["textures"]["rough"];//.value("albedo", undefinedDefault);
	//std::string texMetal = materialDefinition["textures"]["metal"];//.value("albedo", undefinedDefault);
	///*json textureDefObj = json::parse(texturesStr);
	//std::string texAlbedo = textureDefObj.value("albedo", undefinedDefault);*/
	//std::cout << texAlbedo << std::endl;

	//Material* cobbleMat2x = new Material(
	//	vertexShader,
	//	pixelShader,
	//	XMFLOAT4(1, 1, 1, 1),
	//	256.0f,
	//	XMFLOAT2(2, 2),
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions
	//);
	//	//textures[texAlbedo],
	//	//textures[texNorm],
	//	//textures[texRough],
	//	//textures[texMetal],
	//Material* floorMat = new Material(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* paintMat = new Material(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* scratchedMat = new Material(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* bronzeMat = new Material(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2),
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* roughMat = new Material(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2),
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* woodMat = new Material(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);

	////materials.insert({ "cobbleMat2x", cobbleMat2x });
	////materials.insert({ "floorMat", floorMat });
	////materials.insert({ "paintMat", paintMat });
	////materials.insert({ "scratchedMat", scratchedMat });
	////materials.insert({ "bronzeMat", bronzeMat });
	////materials.insert({ "roughMat", roughMat });
	////materials.insert({ "woodMat", woodMat });

	//// Create PBR materials
	//Material* cobbleMat2xPBR = new Material(vertexShader, pixelShaderPBR, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* floorMatPBR = new Material(vertexShader, pixelShaderPBR, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* paintMatPBR = new Material(vertexShader, pixelShaderPBR, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* scratchedMatPBR = new Material(vertexShader, pixelShaderPBR, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* bronzeMatPBR = new Material(vertexShader, pixelShaderPBR, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* roughMatPBR = new Material(vertexShader, pixelShaderPBR, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);
	//Material* woodMatPBR = new Material(vertexShader, pixelShaderPBR, XMFLOAT4(1, 1, 1, 1), 256.0f, XMFLOAT2(2, 2), 
	//	textures["cobblestone_albedo.png"],
	//	textures["cobblestone_normals.png"],
	//	textures["cobblestone_roughness.png"],
	//	textures["cobblestone_metal.png"],
	//	samplerOptions);

	/*materials.insert({ "cobbleMat2xPBR", cobbleMat2xPBR });
	materials.insert({ "floorMatPBR", floorMatPBR });
	materials.insert({ "paintMatPBR", paintMatPBR });
	materials.insert({ "scratchedMatPBR", scratchedMatPBR });
	materials.insert({ "bronzeMatPBR", bronzeMatPBR });
	materials.insert({ "roughMatPBR", roughMatPBR });
	materials.insert({ "woodMatPBR", woodMatPBR });*/

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
