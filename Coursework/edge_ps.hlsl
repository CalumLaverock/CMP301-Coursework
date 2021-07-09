Texture2D textureGray : register(t0);

SamplerState sampler0 : register(s0);

cbuffer ScreenBuffer : register(b0)
{
	int2 screenSize;
	float2 padding;
};

cbuffer ColourBuffer : register(b1)
{
	float4 outlineCol;
	float4 insideCol;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

float4 main(InputType input) : SV_TARGET
{
	// Sobel calculations
	float2 pxSz = float2(1.0f / screenSize.x, 1.0f / screenSize.y);

	//calculate the offsets for the surrounding pixels
	float2 o00 = input.tex + float2(-pxSz.x, -pxSz.y);
	float2 o10 = input.tex + float2(0.f, -pxSz.y);
	float2 o20 = input.tex + float2(pxSz.x, -pxSz.y);
		 
	float2 o01 = input.tex + float2(-pxSz.x, 0.f);
	float2 o21 = input.tex + float2(pxSz.x, 0.f);
		 
	float2 o02 = input.tex + float2(-pxSz.x, pxSz.y);
	float2 o12 = input.tex + float2(0.f, pxSz.y);
	float2 o22 = input.tex + float2(pxSz.x, pxSz.y);

	//sample the greyscale image to later detect edges
	float h00 = textureGray.Sample(sampler0, o00).x;
	float h10 = textureGray.Sample(sampler0, o10).x;
	float h20 = textureGray.Sample(sampler0, o20).x;
		 			   
	float h01 = textureGray.Sample(sampler0, o01).x;
	float h21 = textureGray.Sample(sampler0, o21).x;
		 			   
	float h02 = textureGray.Sample(sampler0, o02).x;
	float h12 = textureGray.Sample(sampler0, o12).x;
	float h22 = textureGray.Sample(sampler0, o22).x;

	//apply the sobel filters
	//	   [-1 0 1]   [h00 h10 h20]
	//Gx = [-2 0 2] * [h01     h21]
	//	   [-1 0 1]   [h02 h12 h22]

	//	   [-1 -2 -1]   [h00 h10 h20]
	//Gy = [ 0  0  0] * [h01     h21]
	//	   [ 1  2  1]   [h02 h12 h22]
	float Gx = h20 + (2 * h21) + h22 - h00 - (2 * h01) - h02;
	float Gy = h02 + (2 * h12) + h22 - h00 - (2 * h10) - h20;

	//calculate a gradient after applying the sobel filter
	float gradient = sqrt(pow(Gx, 2) + pow(Gy, 2));
	
	float4 finalColour;

	//check if pixel is an edge and colour by checking if the gradient is greater than some threshold
	if (gradient > 0.4)
	{
		//if pixel is an edge
		finalColour = outlineCol;
	}
	else
	{
		//if pixel is not an edge
		finalColour = insideCol;
	}

	return finalColour;
}