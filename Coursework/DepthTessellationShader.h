// Colour shader.h
// Simple shader example.
#pragma once

#include "../DXFramework/DXF.h"

using namespace std;
using namespace DirectX;


class DepthTessellationShader : public BaseShader
{
	struct TessellationBufferType
	{
		XMFLOAT4 cam_pos;
	};

public:

	DepthTessellationShader(ID3D11Device* device, HWND hwnd);
	~DepthTessellationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, XMFLOAT4 cam_position);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

private:
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* tessellationBuffer;
};
