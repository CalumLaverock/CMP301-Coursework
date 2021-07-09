// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"
#include "TessellationQuad.h"
#include "TessellationShader.h"
#include "TextureShader.h"
#include "DepthShader.h"
#include "EdgeShader.h"
#include "GrayscaleShader.h"
#include "LightShader.h"


class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	void depthPass();
	void renderPass();
	void grayscalePass();
	void edgePass();
	void finalPass();

	bool render();
	void gui();

private:
	TessellationQuad* floor;
	TessellationShader* tesShader;
	TextureShader* texShader;
	DepthShader* depthShader;
	EdgeShader* edgeShader;
	GrayscaleShader* grayscaleShader;
	LightShader* lightShader;

	Light* directionalLight;
	Light* spotlight;

	Model* turret;
	Model* smallTurrets[2];
	CubeMesh* ammoCrate;
	OrthoMesh* orthoMesh;

	RenderTexture* renderTexture;
	RenderTexture* edgeTexture;
	RenderTexture* grayscaleTexture;

	ShadowMap* shadowMapDir;
	ShadowMap* shadowMapSpot;

	XMINT2 screenSize;

	bool spotOn = true;
	bool dirOn = true;

	float* outlineColour;
	float* insideColour;
	bool edgeDetect = false;
};

#endif