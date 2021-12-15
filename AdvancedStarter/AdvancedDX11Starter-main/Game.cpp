
#include <stdlib.h>     // For seeding random and rand()
#include <time.h>       // For grabbing time (to seed random)

#include "Game.h"
#include "Vertex.h"


#include "WICTextureLoader.h"

#include "imgui/imgui.h"

#include "imgui/imgui_impl_dx11.h" //visual studio wanted me to have these too, why did just the first one not work?
#include "imgui/imgui_impl_win32.h"

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// Helper macro for getting a float between min and max
#define RandomRange(min, max) (float)rand() / RAND_MAX * (max - min) + min

// Helper macros for making texture and shader loading code more succinct
#define LoadTexture(file, srv) CreateWICTextureFromFile(device.Get(), context.Get(), GetFullPathTo_Wide(file).c_str(), 0, srv.GetAddressOf())
#define LoadShader(type, file) new type(device.Get(), context.Get(), GetFullPathTo_Wide(file).c_str())


// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	camera = 0;

	// Seed random
	srand((unsigned int)time(0));

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Note: Since we're using smart pointers (ComPtr),
	// we don't need to explicitly clean up those DirectX objects
	// - If we weren't using smart pointers, we'd need
	//   to call Release() on each DirectX object

	// Clean up our other resources
	//for (auto& m : meshes) delete m;
	//for (auto& s : shaders) delete s; 
	//for (auto& m : materials) delete m;
	//for (auto& e : entities) delete e;

	// Delete any one-off objects
	//delete sky;
	delete camera;
	delete arial;
	delete spriteBatch;
	delete renderer;


	// Delete singletons
	delete& Input::GetInstance();
	delete& AssetManager::GetInstance();

	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();
	// 
	// 
	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());

	// Initialize the input manager with the window's handle
	Input::GetInstance().Initialize(this->hWnd);

	// Asset loading and entity creation
	LoadAssetsAndCreateEntities();

	//initialize mandelbrot data
	mandelbrot = std::make_shared<Mandelbrot>(device, context, width, height, camera);
	
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set up lights initially
	lightCount = 30;
	maxLights = 100;
	minPointLightRange = 5.0f;
	maxPointLightRange = 10.0f;
	minPointLightIntensity = 0.1f;
	maxPointLightIntensity = 3.0f;
	GenerateLights();

	// Make our camera
	camera = new Camera(
		0, 0, -10,	// Position
		3.0f,		// Move speed
		1.0f,		// Mouse look
		this->width / (float)this->height); // Aspect ratio

	CreateTransformHierarchies();
	CreateParticleEmitters();

	currentSceneState = SceneState::Fractal;

	renderer = new Renderer(
		device,
		context,
		swapChain,
		backBufferRTV,
		depthStencilView,
		width,
		height,
		sky,
		entities,
		lights,
		lightMesh,
		lightVS,
		lightPS,
		particleEmitters
	);
}

void Game::CreateTransformHierarchies()
{
	//entities[0]->GetTransform()->AddChild(entities[1]->GetTransform());
	//entities[1]->GetTransform()->AddChild(entities[2]->GetTransform());
	//entities[2]->GetTransform()->AddChild(entities[3]->GetTransform());
	//entities[3]->GetTransform()->AddChild(entities[4]->GetTransform());

	//entities[5]->GetTransform()->AddChild(entities[6]->GetTransform());
	//entities[6]->GetTransform()->AddChild(entities[7]->GetTransform());
	//entities[7]->GetTransform()->AddChild(entities[8]->GetTransform());
	//entities[8]->GetTransform()->AddChild(entities[9]->GetTransform());
	//entities[9]->GetTransform()->AddChild(entities[10]->GetTransform());
}


// --------------------------------------------------------
// Load all assets and create materials, entities, etc.
// --------------------------------------------------------
void Game::LoadAssetsAndCreateEntities()
{
	//load using asset manager
	AssetManager& assetMngr = AssetManager::GetInstance();

	assetMngr.Initialize(
		GetFullPathTo("../../Assets"), 
		GetFullPathTo_Wide(L"../../Assets"), 
		device, 
		context
	);

	assetMngr.LoadVertexShader(
		GetFullPathTo_Wide(L"VertexShader.cso"), 
		"VertexShader.cso"
	);
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"PixelShader.cso"),
		"PixelShader.cso"
	);
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"PixelShaderPBR.cso"),
		"PixelShaderPBR.cso"
	);
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"SolidColorPS.cso"),
		"SolidColorPS.cso"
	);

	//sky shaders
	assetMngr.LoadVertexShader(
		GetFullPathTo_Wide(L"SkyVS.cso"),
		"SkyVS.cso"
	);
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"SkyPS.cso"),
		"SkyPS.cso"
	);

	//shaders for IBL PBR lighting
	assetMngr.LoadVertexShader(
		GetFullPathTo_Wide(L"FullscreenVS.cso"),
		"FullscreenVS.cso"
	);
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"IBLIrradianceMapPS.cso"),
		"IBLIrradianceMapPS.cso"
	);
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"IBLSpecularConvolutionPS.cso"),
		"IBLSpecularConvolutionPS.cso"
	);	
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"IBLBrdfLookUpTablePS.cso"),
		"IBLBrdfLookUpTablePS.cso"
	);

	//pixel shader for refraction
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"RefractionPS.cso"),
		"RefractionPS.cso"
	);
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"SimpleTexturePS.cso"),
		"SimpleTexturePS.cso"
	);
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"CombinePS.cso"),
		"CombinePS.cso"
	);

	//particle shaders
	assetMngr.LoadVertexShader(
		GetFullPathTo_Wide(L"ParticleVS.cso"),
		"ParticleVS.cso"
	);
	assetMngr.LoadPixelShader(
		GetFullPathTo_Wide(L"ParticlePS.cso"),
		"ParticlePS.cso"
	);

	assetMngr.LoadComputeShader(
		GetFullPathTo_Wide(L"FractalCS.cso"),
		"FractalCS.cso"
	);

	//needs to happen after shaders are loaded right now because it relies on the shaders already existing to create the materials and everything
	assetMngr.LoadAllAssets();

	SimpleVertexShader* vertexShader	= assetMngr.GetVertexShader("VertexShader.cso");
	SimplePixelShader* pixelShader		= assetMngr.GetPixelShader("PixelShader.cso");
	SimplePixelShader* pixelShaderPBR	= assetMngr.GetPixelShader("PixelShaderPBR.cso");
	SimplePixelShader* solidColorPS		= assetMngr.GetPixelShader("SolidColorPS.cso");

	SimpleVertexShader* skyVS = assetMngr.GetVertexShader("SkyVS.cso");
	SimplePixelShader* skyPS = assetMngr.GetPixelShader("SkyPS.cso");

	// Set up the sprite batch and load the sprite font
	spriteBatch = new SpriteBatch(context.Get());
	arial = new SpriteFont(device.Get(), GetFullPathTo_Wide(L"../../Assets/Textures/arial.spritefont").c_str());

	// Grab the meshes from the asset manager
	Mesh* sphereMesh = assetMngr.GetMesh("sphere.obj");
	Mesh* helixMesh = assetMngr.GetMesh("helix.obj");
	Mesh* cubeMesh = assetMngr.GetMesh("cube.obj");
	Mesh* coneMesh = assetMngr.GetMesh("cone.obj");

	meshes.push_back(sphereMesh);
	meshes.push_back(helixMesh);
	meshes.push_back(cubeMesh);
	meshes.push_back(coneMesh);


	// grab the sky from the asset manager
	sky = assetMngr.GetSky();


	// grab basic materials from asset manager
	Material* cobbleMat2x  = assetMngr.GetMaterial("cobbleMat2x");
	Material* floorMat =     assetMngr.GetMaterial("floorMat");
	Material* paintMat =     assetMngr.GetMaterial("paintMat");
	Material* scratchedMat = assetMngr.GetMaterial("scratchedMat");
	Material* bronzeMat =    assetMngr.GetMaterial("bronzeMat");
	Material* roughMat =     assetMngr.GetMaterial("roughMat");
	Material* woodMat =      assetMngr.GetMaterial("woodMat");

	materials.push_back(cobbleMat2x);
	materials.push_back(floorMat);
	materials.push_back(paintMat);
	materials.push_back(scratchedMat);
	materials.push_back(bronzeMat);
	materials.push_back(roughMat);
	materials.push_back(woodMat);

	// grab PBR materials from asset manager
	Material* cobbleMat2xPBR =  assetMngr.GetMaterial("cobbleMat2xPBR");
	Material* floorMatPBR =     assetMngr.GetMaterial("floorMatPBR");
	Material* paintMatPBR =     assetMngr.GetMaterial("paintMatPBR");
	Material* scratchedMatPBR = assetMngr.GetMaterial("scratchedMatPBR");
	Material* bronzeMatPBR =    assetMngr.GetMaterial("bronzeMatPBR");
	Material* roughMatPBR =     assetMngr.GetMaterial("roughMatPBR");
	Material* woodMatPBR =      assetMngr.GetMaterial("woodMatPBR");

	materials.push_back(cobbleMat2xPBR);
	materials.push_back(floorMatPBR);
	materials.push_back(paintMatPBR);
	materials.push_back(scratchedMatPBR);
	materials.push_back(bronzeMatPBR);
	materials.push_back(roughMatPBR);
	materials.push_back(woodMatPBR);



	// === Create the PBR entities =====================================
	GameEntity* cobSpherePBR = new GameEntity(sphereMesh, cobbleMat2xPBR);
	cobSpherePBR->GetTransform()->SetScale(2, 2, 2);
	cobSpherePBR->GetTransform()->SetPosition(-6, 2, 0);
	cobSpherePBR->GetMaterial()->SetRefractive(true); //i hate this

	GameEntity* floorSpherePBR = new GameEntity(sphereMesh, floorMatPBR);
	floorSpherePBR->GetTransform()->SetScale(2, 2, 2);
	floorSpherePBR->GetTransform()->SetPosition(-4, 2, 0);
	floorSpherePBR->GetMaterial()->SetRefractive(true); //i hate this

	GameEntity* paintSpherePBR = new GameEntity(sphereMesh, paintMatPBR);
	paintSpherePBR->GetTransform()->SetScale(2, 2, 2);
	paintSpherePBR->GetTransform()->SetPosition(-2, 2, 0);

	GameEntity* scratchSpherePBR = new GameEntity(sphereMesh, scratchedMatPBR);
	scratchSpherePBR->GetTransform()->SetScale(2, 2, 2);
	scratchSpherePBR->GetTransform()->SetPosition(0, 2, 0);

	GameEntity* bronzeSpherePBR = new GameEntity(sphereMesh, bronzeMatPBR);
	bronzeSpherePBR->GetTransform()->SetScale(2, 2, 2);
	bronzeSpherePBR->GetTransform()->SetPosition(2, 2, 0);

	GameEntity* roughSpherePBR = new GameEntity(sphereMesh, roughMatPBR);
	roughSpherePBR->GetTransform()->SetScale(2, 2, 2);
	roughSpherePBR->GetTransform()->SetPosition(4, 2, 0);

	GameEntity* woodSpherePBR = new GameEntity(sphereMesh, woodMatPBR);
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
	GameEntity* cobSphere = new GameEntity(sphereMesh, cobbleMat2x);
	cobSphere->GetTransform()->SetScale(2, 2, 2);
	cobSphere->GetTransform()->SetPosition(-6, -2, 0);
	//cobSphere->GetMaterial()->SetRefractive(true); //i hate this so much

	GameEntity* floorSphere = new GameEntity(sphereMesh, floorMat);
	floorSphere->GetTransform()->SetScale(2, 2, 2);
	floorSphere->GetTransform()->SetPosition(-4, -2, 0);

	GameEntity* paintSphere = new GameEntity(sphereMesh, paintMat);
	paintSphere->GetTransform()->SetScale(2, 2, 2);
	paintSphere->GetTransform()->SetPosition(-2, -2, 0);

	GameEntity* scratchSphere = new GameEntity(sphereMesh, scratchedMat);
	scratchSphere->GetTransform()->SetScale(2, 2, 2);
	scratchSphere->GetTransform()->SetPosition(0, -2, 0);

	GameEntity* bronzeSphere = new GameEntity(sphereMesh, bronzeMat);
	bronzeSphere->GetTransform()->SetScale(2, 2, 2);
	bronzeSphere->GetTransform()->SetPosition(2, -2, 0);

	GameEntity* roughSphere = new GameEntity(sphereMesh, roughMat);
	roughSphere->GetTransform()->SetScale(2, 2, 2);
	roughSphere->GetTransform()->SetPosition(4, -2, 0);

	GameEntity* woodSphere = new GameEntity(sphereMesh, woodMat);
	woodSphere->GetTransform()->SetScale(2, 2, 2);
	woodSphere->GetTransform()->SetPosition(6, -2, 0);

	entities.push_back(cobSphere);
	entities.push_back(floorSphere);
	entities.push_back(paintSphere);
	entities.push_back(scratchSphere);
	entities.push_back(bronzeSphere);
	entities.push_back(roughSphere);
	entities.push_back(woodSphere);


	//plain material entites to show IBL obviously
	Material* solidMetalMatPBR1 = assetMngr.GetMaterial("solidMetalMatPBR1");
	Material* solidMetalMatPBR2 = assetMngr.GetMaterial("solidMetalMatPBR2");
	Material* solidMetalMatPBR3 = assetMngr.GetMaterial("solidMetalMatPBR3");
	Material* solidMetalMatPBR4 = assetMngr.GetMaterial("solidMetalMatPBR4");
	Material* solidMetalMatPBR5 = assetMngr.GetMaterial("solidMetalMatPBR5");


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

	

	// Save assets needed for drawing point lights
	// (Since these are just copies of the pointers,
	//  we won't need to directly delete them as 
	//  the original pointers will be cleaned up)
	lightMesh = sphereMesh;
	lightVS = vertexShader;
	lightPS = solidColorPS;
}




void Game::CreateParticleEmitters()
{
	//grab assets from asset manager
	AssetManager& assetMngr = AssetManager::GetInstance();
	//std::shared_ptr<SimpleVertexShader> vs = std::make_shared<SimpleVertexShader>(assetMngr.GetVertexShader("ParticleVS.cso"));
	//std::shared_ptr<SimplePixelShader> ps = std::make_shared<SimplePixelShader>(assetMngr.GetPixelShader("ParticlePS.cso"));
	SimpleVertexShader* vs = assetMngr.GetVertexShader("ParticleVS.cso");
	SimplePixelShader* ps = assetMngr.GetPixelShader("ParticlePS.cso");

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture1 = assetMngr.GetTexture("dirt_03.png");
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture2 = assetMngr.GetTexture("light_03.png");
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture3 = assetMngr.GetTexture("twirl_03.png");
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture4 = assetMngr.GetTexture("star_07.png");
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture5 = assetMngr.GetTexture("spark_02.png");

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


}

// --------------------------------------------------------
// Generates the lights in the scene: 3 directional lights
// and many random point lights.
// --------------------------------------------------------
void Game::GenerateLights()
{
	// Reset
	lights.clear();

	// Setup directional lights
	Light dir1 = {};
	dir1.Type = LIGHT_TYPE_DIRECTIONAL;
	dir1.Direction = XMFLOAT3(1, -1, 1);
	dir1.Color = XMFLOAT3(0.8f, 0.8f, 0.8f);
	dir1.Intensity = 1.0f;

	Light dir2 = {};
	dir2.Type = LIGHT_TYPE_DIRECTIONAL;
	dir2.Direction = XMFLOAT3(-1, -0.25f, 0);
	dir2.Color = XMFLOAT3(0.2f, 0.2f, 0.2f);
	dir2.Intensity = 1.0f;

	Light dir3 = {};
	dir3.Type = LIGHT_TYPE_DIRECTIONAL;
	dir3.Direction = XMFLOAT3(0, -1, 1);
	dir3.Color = XMFLOAT3(0.2f, 0.2f, 0.2f);
	dir3.Intensity = 1.0f;

	// Add light to the list
	lights.push_back(dir1);
	lights.push_back(dir2);
	lights.push_back(dir3);

	// Create the rest of the lights
	while (lights.size() < lightCount)
	{
		CreateRandomPointLight();
	}

}

void Game::CreateRandomPointLight()
{
	Light point = {};
	point.Type = LIGHT_TYPE_POINT;
	point.Position = XMFLOAT3(RandomRange(-10.0f, 10.0f), RandomRange(-5.0f, 5.0f), RandomRange(-10.0f, 10.0f));
	point.Color = XMFLOAT3(RandomRange(0, 1), RandomRange(0, 1), RandomRange(0, 1));
	point.Range = RandomRange(minPointLightRange, maxPointLightRange);
	point.Intensity = RandomRange(minPointLightIntensity, maxPointLightIntensity);

	// Add to the list
	lights.push_back(point);
}



// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update our projection matrix to match the new aspect ratio
	if (camera)
		camera->UpdateProjectionMatrix(this->width / (float)this->height);

	//update renderer data
	renderer->PostResize(width, height, backBufferRTV, depthStencilView);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{

	Input& input = Input::GetInstance();
	HandleGuiUpdate(deltaTime, input);


	switch (currentSceneState)
	{
	case SceneState::Main:
		// Update the camera
		camera->Update(deltaTime);

		// Check individual input
		if (input.KeyDown(VK_ESCAPE)) Quit();
		if (input.KeyPress(VK_TAB)) GenerateLights();

		//If lightCount was increased by user, create more random lights
		while (lights.size() < lightCount)
		{
			CreateRandomPointLight();
		}
		renderer->UpdateLightVec(lights); //i hate this, please just use shared pointers already

		//UpdateEntitityTransforms();

		for (auto emit : particleEmitters)
		{
			emit->Update(deltaTime, totalTime);
		}

		break;
	case SceneState::Fractal:

		break;
	default:
		break;
	}
}

void Game::UpdateEntitityTransforms()
{
	entities[0]->GetTransform()->MoveRelative(0.01f, 0.0f, 0.0f);
	entities[0]->GetTransform()->Rotate(0.01f, 0.0f, 0.0f);

	entities[6]->GetTransform()->Rotate(0.0f, 0.001f, 0.001f);
	entities[6]->GetTransform()->SetScale(0.5f, 0.5f, 0.5f);

}

void Game::HandleGuiUpdate(float deltaTime, Input& input)
{
	// Reset input manager's gui state so we don’t
		// taint our own input (you’ll uncomment later)
	input.SetGuiKeyboardCapture(false);
	input.SetGuiMouseCapture(false);
	// Set io info
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->width;
	io.DisplaySize.y = (float)this->height;
	io.KeyCtrl = input.KeyDown(VK_CONTROL);
	io.KeyShift = input.KeyDown(VK_SHIFT);
	io.KeyAlt = input.KeyDown(VK_MENU);
	io.MousePos.x = (float)input.GetMouseX();
	io.MousePos.y = (float)input.GetMouseY();
	io.MouseDown[0] = input.MouseLeftDown();
	io.MouseDown[1] = input.MouseRightDown();
	io.MouseDown[2] = input.MouseMiddleDown();
	io.MouseWheel = input.GetMouseWheel();
	input.GetKeyArray(io.KeysDown, 256);
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture (you’ll uncomment later)
	input.SetGuiKeyboardCapture(io.WantCaptureKeyboard);
	input.SetGuiMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	// Demo website also here: https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html
	//ImGui::ShowDemoWindow();


	switch (currentSceneState)
	{
	case SceneState::Main:
		ShowEngineStats(io.Framerate);
		ShowLightsEditor();
		ShowRenderTargets();

		break;
	case SceneState::Fractal:

		break;
	default:
		break;
	}
	ShowSceneSelect();
}

void Game::ShowSceneSelect()
{
	if (ImGui::TreeNode("Select Scene"))
	{
		//i thought this would be good to have as an enum select but this would probably be more robust and dynamic if it was an array instead
		char buf[32];
		sprintf_s(buf, "Main");
		if (ImGui::Selectable(buf, currentSceneState == SceneState::Main))
			currentSceneState = SceneState::Main;		
		//char buf[32];
		sprintf_s(buf, "Fractal");
		if (ImGui::Selectable(buf, currentSceneState == SceneState::Fractal))
			currentSceneState = SceneState::Fractal;
		//static int selected = -1;
		//for (int n = 0; n < int(SceneState::COUNT); n++)
		//{
		//	if (ImGui::Selectable(buf, selected == n))
		//		selected = n;
		//}
		ImGui::TreePop();
	}
}

void Game::ShowLightsEditor()
{
	//lights editor
	ImGui::Begin("Lights");

	//slider to change how many of them actually render
	ImGui::SliderInt("Lights in Scene", &lightCount, 0, maxLights, "%d");


	//display lights info as tree
	if (ImGui::CollapsingHeader("Lights"))
	{
		for (int i = 0; i < lightCount; i++)
		{
			if (ImGui::TreeNode((void*)(intptr_t)i, "Light %d", i))
			{
				AddLabeledInt("Type: ", lights[i].Type); //id how to convert that to the actual text right now
				if (lights[i].Type == LIGHT_TYPE_DIRECTIONAL) //if its not directional, then theres no direction to display (unless maybe spotlight but shhh)
				{
					ImGui::DragFloat3("Direction: ", &lights[i].Direction.x);
				}
				if (lights[i].Type != LIGHT_TYPE_DIRECTIONAL) //Directional lights have no position
				{
					ImGui::SliderFloat("Range", &lights[i].Range, minPointLightRange, maxPointLightRange);
					ImGui::DragFloat3("Position: ", &lights[i].Position.x);
					ImGui::SliderFloat("Intensity", &lights[i].Intensity, minPointLightIntensity, maxPointLightIntensity);
					//AddLabeledFloat("SpotFalloff: ", lights[i].SpotFalloff); //isnt this just for spotlights? we dont have any of those right now
				}
				ImGui::DragFloat3("Color: ", &lights[i].Color.x);

				/*ImGui::SameLine();
				if (ImGui::SmallButton("button")) {}*/
				ImGui::TreePop();
			}
		}
	}

	ImGui::End();
}

void Game::ShowEngineStats(float framerate)
{
	ImGui::Begin("Engine Stats");

	//display FPS
	AddLabeledInt("FPS: ", framerate); //automatically truncate the decimal part, makes this less of an eyesore

	//display window dimensions
	AddLabeledInt("Window Height: ", height);
	AddLabeledInt("Window Width: ", width);

	//display entities
	AddLabeledInt("Entities: ", entities.size());


	ImGui::End();
}

void Game::ShowRenderTargets()
{
	ImGui::Begin("Render Targets");


	if (ImGui::CollapsingHeader("All Render Targets"))
	{
		ImVec2 size = ImGui::GetItemRectSize();
		float rtHeight = size.x * ((float)height / width);

		ImGui::Image(renderer->GetSceneColorsSRV().Get(), ImVec2(500, 300));
		ImGui::Image(renderer->GetSceneNormalsSRV().Get(), ImVec2(500, 300));
		ImGui::Image(renderer->GetSceneDepthsSRV().Get(), ImVec2(500, 300));
		ImGui::Image(renderer->GetRefractionSilhouetteSRV().Get(), ImVec2(500, 300));
		ImGui::Image(renderer->GetFinalCompositeSRV().Get(), ImVec2(500, 300));
	}

	ImGui::End();
}

void Game::ShowFractalInfo()
{
}

//there must be a cleaner way than having two methods that are basically the same other than the parameters, right?
void Game::AddLabeledFloat(std::string label, float value)
{
	std::string valueStr = std::to_string(value);
	ConcatAndCreateText(label, valueStr);
}

void Game::AddLabeledInt(std::string label, int value)
{
	std::string valueStr = std::to_string(value);
	ConcatAndCreateText(label, valueStr);
}

void Game::ConcatAndCreateText(std::string& label, std::string& valueStr)
{
	std::string valueDisplay = label + valueStr;
	ImGui::Text(valueDisplay.c_str());
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	mandelbrot->RunComputeShader();
	renderer->Render(camera, lightCount, totalTime, currentSceneState);
	//can get rid of this switch i think???? 
	switch (currentSceneState)
	{
	case SceneState::Main:
		break;
	case SceneState::Fractal:

		break;
	default:
		break;
	}
}


// --------------------------------------------------------
// Draws the point lights as solid color spheres
// --------------------------------------------------------
void Game::DrawPointLights()
{
	// Turn on these shaders
	lightVS->SetShader();
	lightPS->SetShader();

	// Set up vertex shader
	lightVS->SetMatrix4x4("view", camera->GetView());
	lightVS->SetMatrix4x4("projection", camera->GetProjection());

	for (int i = 0; i < lightCount; i++)
	{
		Light light = lights[i];

		// Only drawing points, so skip others
		if (light.Type != LIGHT_TYPE_POINT)
			continue;

		// Calc quick scale based on range
		// (assuming range is between 5 - 10)
		float scale = light.Range / 10.0f;

		// Make the transform for this light
		XMMATRIX rotMat = XMMatrixIdentity();
		XMMATRIX scaleMat = XMMatrixScaling(scale, scale, scale);
		XMMATRIX transMat = XMMatrixTranslation(light.Position.x, light.Position.y, light.Position.z);
		XMMATRIX worldMat = scaleMat * rotMat * transMat;

		XMFLOAT4X4 world;
		XMFLOAT4X4 worldInvTrans;
		XMStoreFloat4x4(&world, worldMat);
		XMStoreFloat4x4(&worldInvTrans, XMMatrixInverse(0, XMMatrixTranspose(worldMat)));

		// Set up the world matrix for this light
		lightVS->SetMatrix4x4("world", world);
		lightVS->SetMatrix4x4("worldInverseTranspose", worldInvTrans);

		// Set up the pixel shader data
		XMFLOAT3 finalColor = light.Color;
		finalColor.x *= light.Intensity;
		finalColor.y *= light.Intensity;
		finalColor.z *= light.Intensity;
		lightPS->SetFloat3("Color", finalColor);

		// Copy data
		lightVS->CopyAllBufferData();
		lightPS->CopyAllBufferData();

		// Draw
		lightMesh->SetBuffersAndDraw(context);
	}

}


// --------------------------------------------------------
// Draws a simple informational "UI" using sprite batch
// --------------------------------------------------------
void Game::DrawUI()
{
	spriteBatch->Begin();

	// Basic controls
	float h = 10.0f;
	arial->DrawString(spriteBatch, L"Controls:", XMVectorSet(10, h, 0, 0));
	arial->DrawString(spriteBatch, L" (WASD, X, Space) Move camera", XMVectorSet(10, h + 20, 0, 0));
	arial->DrawString(spriteBatch, L" (Left Click & Drag) Rotate camera", XMVectorSet(10, h + 40, 0, 0));
	arial->DrawString(spriteBatch, L" (Left Shift) Hold to speed up camera", XMVectorSet(10, h + 60, 0, 0));
	arial->DrawString(spriteBatch, L" (Left Ctrl) Hold to slow down camera", XMVectorSet(10, h + 80, 0, 0));
	arial->DrawString(spriteBatch, L" (TAB) Randomize lights", XMVectorSet(10, h + 100, 0, 0));

	// Current "scene" info
	h = 150;
	arial->DrawString(spriteBatch, L"Scene Details:", XMVectorSet(10, h, 0, 0));
	arial->DrawString(spriteBatch, L" Top: PBR materials", XMVectorSet(10, h + 20, 0, 0));
	arial->DrawString(spriteBatch, L" Bottom: Non-PBR materials", XMVectorSet(10, h + 40, 0, 0));

	spriteBatch->End();

	// Reset render states, since sprite batch changes these!
	context->OMSetBlendState(0, 0, 0xFFFFFFFF);
	context->OMSetDepthStencilState(0, 0);

}
