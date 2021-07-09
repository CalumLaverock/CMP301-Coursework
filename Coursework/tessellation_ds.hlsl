// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
	matrix lightViewMatrix[2];
	matrix lightProjectionMatrix[2];
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct InputType
{
    float3 position : POSITION;
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

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    float3 vertexPosition;
	float3 vertexNormal;
	float2 vertexTex;
    OutputType output;
 
    // Determine the position of the new vertex.
	// Invert the y and Z components of uvwCoord as these coords are generated in UV space and therefore y is positive downward.
	// Alternatively you can set the output topology of the hull shader to cw instead of ccw (or vice versa).
	float3 posComp1 = lerp(patch[0].position, patch[1].position, uvwCoord.y); 
	float3 posComp2 = lerp(patch[3].position, patch[2].position, uvwCoord.y);
	vertexPosition = lerp(posComp1, posComp2, uvwCoord.x);

	float3 normComp1 = lerp(patch[0].normal, patch[1].normal, uvwCoord.y);
	float3 normComp2 = lerp(patch[3].normal, patch[2].normal, uvwCoord.y);
	vertexNormal = lerp(normComp1, normComp2, uvwCoord.x);
	output.normal = vertexNormal;

	float2 texComp1 = lerp(patch[0].tex, patch[1].tex, uvwCoord.y);
	float2 texComp2 = lerp(patch[3].tex, patch[2].tex, uvwCoord.y);
	vertexTex = lerp(texComp1, texComp2, uvwCoord.x);
	output.tex = vertexTex;

	//Move the vertex along its normal by a value based on the height map's colour * 15
	float4 textureColour = texture0.SampleLevel(sampler0, output.tex, 0);
	float offset = textureColour.x * 15;
	vertexPosition += offset * output.normal;

	// Sobel normal calculations
	// Sobel implementation used from:
	// Practical Rendering and Computation with Direct3D 11 book by Jack Hoxley, Jason Zink, and Matt Pettineo
	float2 pxSz = float2(1.0f / 2048, 1.0f / 2048);

	//calculate the offsets for the surrounding pixels
	float2 o00 = output.tex + float2(-pxSz.x, -pxSz.y);
	float2 o10 = output.tex + float2(0.f, -pxSz.y);
	float2 o20 = output.tex + float2(pxSz.x, -pxSz.y);
			   
	float2 o01 = output.tex + float2(-pxSz.x, 0.f);
	float2 o11 = output.tex + float2(pxSz.x, 0.f);
			   
	float2 o02 = output.tex + float2(-pxSz.x, pxSz.y);
	float2 o12 = output.tex + float2(0.f, pxSz.y);
	float2 o22 = output.tex + float2(pxSz.x, pxSz.y);

	//sample the texture for the surrounding pixels
	float h00 = texture0.SampleLevel(sampler0, o00, 0);
	float h10 = texture0.SampleLevel(sampler0, o10, 0);
	float h20 = texture0.SampleLevel(sampler0, o20, 0);

	float h01 = texture0.SampleLevel(sampler0, o01, 0);
	float h11 = texture0.SampleLevel(sampler0, o11, 0);

	float h02 = texture0.SampleLevel(sampler0, o02, 0);
	float h12 = texture0.SampleLevel(sampler0, o12, 0);
	float h22 = texture0.SampleLevel(sampler0, o22, 0);

	//apply the sobel filters
	float Gx = h00 - h20 + 2.0f * h01 - 2.0f * h11 + h02 - h22;
	float Gy = h00 + 2.0f * h10 + h20 - h02 - 2.0f * h12 - h22;

	float Gz = 0.01f * sqrt(max(0.f, 1.0f - Gx * Gx - Gy * Gy));

	output.normal = float3(2.0f * Gx, Gz, 2.0f * Gy);

    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	output.depthPosition = output.position;

	// Calculate the vertex position from the directional light's point of view
	output.lightViewPos[0] = mul(float4(vertexPosition, 1), worldMatrix);
	output.lightViewPos[0] = mul(output.lightViewPos[0], lightViewMatrix[0]);
	output.lightViewPos[0] = mul(output.lightViewPos[0], lightProjectionMatrix[0]);
	
	// Calculate the vertex position from the spotlight's point of view
	output.lightViewPos[1] = mul(float4(vertexPosition, 1), worldMatrix);
	output.lightViewPos[1] = mul(output.lightViewPos[1], lightViewMatrix[1]);
	output.lightViewPos[1] = mul(output.lightViewPos[1], lightProjectionMatrix[1]);

	output.worldPosition = mul(vertexPosition, worldMatrix).xyz;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(output.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

    return output;
}

