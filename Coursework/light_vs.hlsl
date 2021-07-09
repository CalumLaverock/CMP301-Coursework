// light vertex shader
// Basic shader for rendering textured geometry

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix lightViewMatrix[2];
	matrix lightProjectionMatrix[2];
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 lightViewPos[2] : TEXCOORD1;
	float4 depthPosition : TEXCOORD3;
	float3 worldPosition : TEXCOORD4;
};

OutputType main(InputType input)
{
	OutputType output;

	// Calculate the position of the new vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.depthPosition = output.position;

	// Calculate the vertex position from the directional light's point of view
	output.lightViewPos[0] = mul(input.position, worldMatrix);
	output.lightViewPos[0] = mul(output.lightViewPos[0], lightViewMatrix[0]);
	output.lightViewPos[0] = mul(output.lightViewPos[0], lightProjectionMatrix[0]);

	// Calculate the vertex position from the spotlight's point of view
	output.lightViewPos[1] = mul(input.position, worldMatrix);
	output.lightViewPos[1] = mul(output.lightViewPos[1], lightViewMatrix[1]);
	output.lightViewPos[1] = mul(output.lightViewPos[1], lightProjectionMatrix[1]);

	output.worldPosition = mul(input.position.xyz, worldMatrix).xyz;

	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

	return output;
}