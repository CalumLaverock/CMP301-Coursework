// Colour shader.h
// Simple shader example.
#pragma once

#include "../DXFramework/DXF.h"

using namespace std;
using namespace DirectX;


class EdgeShader : public BaseShader
{
	struct ScreenBufferType
	{
		XMINT2 screenSize;
		XMFLOAT2 padding;
	};

	struct ColourBufferType
	{
		XMFLOAT4 outline;
		XMFLOAT4 inside;
	};

public:

	EdgeShader(ID3D11Device* device, HWND hwnd);
	~EdgeShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* grayTexture, XMINT2 screenDimensions, float* outline, float* inside);
private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* screenBuffer;
	ID3D11Buffer* colourBuffer;
	ID3D11SamplerState* sampleState;
};
