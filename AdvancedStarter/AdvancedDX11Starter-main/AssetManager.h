#pragma once
#include <string>

#include <d3d11.h> // for ID3D11Device and ID3D11DeviceContext
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <unordered_map>
#include <filesystem>
//#include <experimental/filesystem>


#include "Mesh.h"
#include "GameEntity.h"
#include "Sky.h"

#include "SimpleShader.h"
#include "DXCore.h"
class AssetManager
{
#pragma region Singleton
public:
	// Gets the one and only instance of this class
	static AssetManager& GetInstance()
	{
		if (!instance)
		{
			instance = new AssetManager();
		}

		return *instance;
	}
	// Remove these functions (C++ 11 version)
	AssetManager(AssetManager const&) = delete;
	void operator=(AssetManager const&) = delete;

private:
	static AssetManager* instance;
	AssetManager() {};

	//for ImGui
	bool guiWantsKeyboard;
	bool guiWantsMouse;

#pragma endregion

public:
	~AssetManager();
	void Initialize(
		std::string filepath, 
		std::wstring filepathW, 
		Microsoft::WRL::ComPtr<ID3D11Device> device, 
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	);
	void LoadAllAssets();
	void LoadVertexShader(std::wstring filepath, std::string shaderName);
	void LoadPixelShader(std::wstring filepath, std::string shaderName);

	Mesh* GetMesh(std::string name);
	Material* GetMaterial(std::string name);
	SimpleVertexShader* GetVertexShader(std::string name);
	SimplePixelShader* GetPixelShader(std::string name);
	Sky* GetSky();

private:
	Microsoft::WRL::ComPtr<ID3D11Device>		device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context;
	std::wstring assetsFolderPath_Wide;
	std::string assetsFolderPath;

	//important for textures
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerClampOptions;

	//lookup maps for assets
	std::unordered_map<std::string, Mesh*> meshes;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textures;
	std::unordered_map<std::string, Material*> materials;
	std::unordered_map<std::string, SimpleVertexShader*> vertexShaders;
	std::unordered_map<std::string, SimplePixelShader*> pixelShaders;


	Sky* sky;

};

