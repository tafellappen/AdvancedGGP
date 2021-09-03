#pragma once

#include "DXCore.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "SpriteFont.h"
#include "SpriteBatch.h"
#include "Lights.h"
#include "Sky.h"

#include "Input.h"//not sure if this is safe here but i need this for the HandleGuiUpdate method. probably a safer way to do this with a using statement or whatever cpp equivalent

#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <vector>

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	//void HandleGuiUpdate(float deltaTime);

private:

	// Input and mesh swapping
	byte keys[256];
	byte prevKeys[256];

	// Keep track of "stuff" to clean up
	std::vector<Mesh*> meshes;
	std::vector<Material*> materials;
	std::vector<GameEntity*>* currentScene;
	std::vector<GameEntity*> entities;
	std::vector<GameEntity*> entitiesRandom;
	std::vector<GameEntity*> entitiesLineup;
	std::vector<GameEntity*> entitiesGradient;
	std::vector<ISimpleShader*> shaders;
	Camera* camera;

	// Lights
	std::vector<Light> lights;
	int lightCount;

	//GUI things for lights
	int maxLights;
	float minPointLightRange;
	float maxPointLightRange;
	float minPointLightIntensity;
	float maxPointLightIntensity;

	bool allLightsSameColor;
	DirectX::XMFLOAT3 everyLightColor;

	// These will be loaded along with other assets and
	// saved to these variables for ease of access
	Mesh* lightMesh;
	SimpleVertexShader* lightVS;
	SimplePixelShader* lightPS;

	// Text & ui
	DirectX::SpriteFont* arial;
	DirectX::SpriteBatch* spriteBatch;

	// Texture related resources
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;

	// Skybox
	Sky* sky;

	// General helpers for setup and drawing
	void GenerateLights();
	void CreateRandomPointLight();
	void DrawPointLights();
	void DrawUI();

	// Initialization helper method
	void LoadAssetsAndCreateEntities();

	//GUI stuff
	void HandleGuiUpdate(float deltaTime, Input& input); //for ImGui
	void ShowEngineStats(float framerate);

	void AddLabeledFloat(std::string label, float value);

	void ConcatAndCreateText(std::string& label, std::string& valueStr);

	void AddLabeledInt(std::string label, int value);

	//int currentFPS;
};

