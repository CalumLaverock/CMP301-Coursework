// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);
	
	// Intiliase meshes and models
	floor = new TessellationQuad(renderer->getDevice(), renderer->getDeviceContext());
	turret = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/models/turret/source/turret-model.obj");

	for (int i = 0; i < 2; i++)
	{
		smallTurrets[i] = new Model(renderer->getDevice(), renderer->getDeviceContext(), "res/models/turret/source/turret-model.obj");
	}

	ammoCrate = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());

	// Load textures
	textureMgr->loadTexture(L"turret", L"res/models/turret/textures/turret-texture.png");
	textureMgr->loadTexture(L"height_map", L"res/textures/height-map-blurred.png");
	textureMgr->loadTexture(L"ground", L"res/textures/portal-ground-grass.png");
	textureMgr->loadTexture(L"ammo", L"res/textures/ammo-crate-texture.jpg");

	// Initalise shaders
	tesShader = new TessellationShader(renderer->getDevice(), hwnd);
	texShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	edgeShader = new EdgeShader(renderer->getDevice(), hwnd);
	grayscaleShader = new GrayscaleShader(renderer->getDevice(), hwnd);
	lightShader = new LightShader(renderer->getDevice(), hwnd);

	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);
	edgeTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	grayscaleTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	// Initialise scene variables
	int shadowmapWidth = 8192;
	int shadowmapHeight = 8192;
	int sceneWidth = 300;
	int sceneHeight = 300;

	outlineColour = new float[4]{ 1.f, 1.f, 1.f, 1.f };
	insideColour = new float[4]{ 0.f, 0.f, 0.f, 1.f };

	screenSize = XMINT2{ screenWidth, screenHeight };

	// Initialise shadow maps
	shadowMapDir = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
	shadowMapSpot = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	// Initialise lights
	directionalLight = new Light;
	directionalLight->setAmbientColour(0.2f, 0.2f, 0.2f, 1.0f);
	directionalLight->setDiffuseColour(0.6f, 0.6f, 0.6f, 1.0f);
	directionalLight->setDirection(1.0f, -1.0f, 0.0f);
	directionalLight->setPosition(-5.f, 40.f, 50.f);
	directionalLight->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	spotlight = new Light;
	spotlight->setDiffuseColour(1.0f, 0.9f, 0.0f, 1.0f);
	spotlight->setDirection(1.0f, -1.0f, 0.0f);
	spotlight->setPosition(50.0f, 7.5f, 50.0f);
	spotlight->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	camera->setPosition(50.f, 10.f, 0.f);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
}		   

bool App1::frame()
{
	bool result;

	spotlight->setPosition(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);

	XMVECTOR spotDirection = XMVECTOR{ 0.f, 0.f, 1.f, 1.f };
	XMFLOAT3 spotDirectionFloat;
	float roll, pitch, yaw;
	//Turn the camera's rotation values from degress to radians by multiplying by pi/180
	pitch = camera->getRotation().x * 0.0175432f;
	yaw = camera->getRotation().y * 0.0175432f;
	roll = camera->getRotation().z * 0.0175432f;

	XMMATRIX spotRotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	spotDirection = XMVector3TransformCoord(spotDirection, spotRotation);

	XMStoreFloat3(&spotDirectionFloat, spotDirection);
	spotlight->setDirection(spotDirectionFloat.x, spotDirectionFloat.y, spotDirectionFloat.z);

	if (spotOn)
	{
		spotlight->setDiffuseColour(1.f, 0.9f, 0.f, 1.f);
	}
	else
	{
		spotlight->setDiffuseColour(0.f, 0.f, 0.f, 0.f);
	}

	if (dirOn)
	{
		directionalLight->setDiffuseColour(0.6f, 0.6f, 0.6f, 1.f);
	}
	else
	{
		directionalLight->setDiffuseColour(0.f, 0.f, 0.f, 0.f);
	}

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	depthPass();

	renderPass();

	if (edgeDetect)
	{
		grayscalePass();

		edgePass();

		finalPass();
	}

	gui();

	renderer->endScene();

	return true;
}

void App1::depthPass()
{
	XMMATRIX lightViewMatrix;
	XMMATRIX lightProjectionMatrix;
	XMMATRIX worldMatrix;
	XMMATRIX scaleMatrix;
	float xOff = 10.f, yOff = 4.25f, zOff = 10.f;

	//Generate shadow map for directional light
	shadowMapDir->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	directionalLight->generateViewMatrix();
	lightViewMatrix = directionalLight->getViewMatrix();
	lightProjectionMatrix = directionalLight->getOrthoMatrix();
	worldMatrix = renderer->getWorldMatrix();

	floor->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), floor->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(50.f, 4.25f, 65.f);
	scaleMatrix = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	turret->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), turret->getIndexCount());

	for (int i = 0; i < 2; i++)
	{
		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(xOff, yOff, zOff);
		scaleMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
		worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
		smallTurrets[i]->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), smallTurrets[i]->getIndexCount());

		xOff += 60.f;
		zOff += 40.f;
	}

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(23.f, 6.1f, 76.f);
	scaleMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	ammoCrate->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), ammoCrate->getIndexCount());

	//Generate shadow map for spotlight
	xOff = 10.f, yOff = 4.25f, zOff = 10.f;
	shadowMapSpot->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	spotlight->generateViewMatrix();
	lightViewMatrix = spotlight->getViewMatrix();
	lightProjectionMatrix = spotlight->getOrthoMatrix();
	worldMatrix = renderer->getWorldMatrix();

	floor->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), floor->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(50.f, 4.25f, 65.f);
	scaleMatrix = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	turret->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), turret->getIndexCount());

	for (int i = 0; i < 2; i++)
	{
		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(xOff, yOff, zOff);
		scaleMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
		worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
		smallTurrets[i]->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
		depthShader->render(renderer->getDeviceContext(), smallTurrets[i]->getIndexCount());

		xOff += 60.f;
		zOff += 40.f;
	}

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(23.f, 6.1f, 76.f);
	scaleMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	ammoCrate->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), ammoCrate->getIndexCount());

	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::renderPass()
{
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
	XMMATRIX worldMatrix;
	XMMATRIX scaleMatrix;

	//Draw the entire scene to a render texture in order to apply grayscale to it in the next pass
	float xOff = 10.f, yOff = 4.25f, zOff = 10.f;
	if (edgeDetect)
	{
		renderTexture->setRenderTarget(renderer->getDeviceContext());
		renderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.f, 0.f, 1.f, 1.f);
	}
	else
	{
		renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	}

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	floor->sendData(renderer->getDeviceContext());
	tesShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"height_map"), textureMgr->getTexture(L"ground"), shadowMapDir->getDepthMapSRV(), shadowMapSpot->getDepthMapSRV(), XMFLOAT4(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z, 1), directionalLight, spotlight);
	tesShader->render(renderer->getDeviceContext(), floor->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(50.f, 4.25f, 65.f);
	scaleMatrix = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	turret->sendData(renderer->getDeviceContext());
	lightShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"turret"), shadowMapDir->getDepthMapSRV(), shadowMapSpot->getDepthMapSRV(), directionalLight, spotlight);
	lightShader->render(renderer->getDeviceContext(), turret->getIndexCount());

	for (int i = 0; i < 2; i++)
	{
		worldMatrix = renderer->getWorldMatrix();
		worldMatrix = XMMatrixTranslation(xOff, yOff, zOff);
		scaleMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
		worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
		smallTurrets[i]->sendData(renderer->getDeviceContext());
		lightShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"turret"), shadowMapDir->getDepthMapSRV(), shadowMapSpot->getDepthMapSRV(), directionalLight, spotlight);
		lightShader->render(renderer->getDeviceContext(), smallTurrets[i]->getIndexCount());

		xOff += 60.f;
		zOff += 40.f;
	}

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(23.f, 6.1f, 76.f);
	scaleMatrix = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	worldMatrix = XMMatrixMultiply(scaleMatrix, worldMatrix);
	ammoCrate->sendData(renderer->getDeviceContext());
	lightShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"ammo"), shadowMapDir->getDepthMapSRV(), shadowMapSpot->getDepthMapSRV(), directionalLight, spotlight);
	lightShader->render(renderer->getDeviceContext(), ammoCrate->getIndexCount());

	if (edgeDetect)
	{
		renderer->setBackBufferRenderTarget();
	}
}

void App1::grayscalePass()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	grayscaleTexture->setRenderTarget(renderer->getDeviceContext());
	grayscaleTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = grayscaleTexture->getOrthoMatrix();

	// Render for grayscale
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	grayscaleShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, renderTexture->getShaderResourceView());
	grayscaleShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::edgePass()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	edgeTexture->setRenderTarget(renderer->getDeviceContext());
	edgeTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = edgeTexture->getOrthoMatrix();

	// Render for FXAA
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	edgeShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, grayscaleTexture->getShaderResourceView(), screenSize, outlineColour, insideColour);
	edgeShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	renderer->setZBuffer(false);
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	orthoMesh->sendData(renderer->getDeviceContext());
	texShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, edgeTexture->getShaderResourceView());
	texShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);
}

void App1::gui()
{
	std::string spotlightOn = "Spotlight = ", directionalOn = "Directional Light = ";

	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::NewLine();
	spotlightOn += spotOn ? "On" : "Off";
	ImGui::Checkbox(spotlightOn.c_str(), &spotOn);

	ImGui::NewLine();
	directionalOn += dirOn ? "On" : "Off";
	ImGui::Checkbox(directionalOn.c_str(), &dirOn);

	ImGui::Checkbox("Edge Detection", &edgeDetect);
	if (edgeDetect)
	{
		ImGui::NewLine();
		ImGui::ColorEdit4("Outline Colour", outlineColour);

		ImGui::NewLine();
		ImGui::ColorEdit4("Inside Colour", insideColour);
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

