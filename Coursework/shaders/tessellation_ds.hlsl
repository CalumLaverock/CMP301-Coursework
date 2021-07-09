// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
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
};

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    float3 vertexPosition;
	float3 vertexNormal;
	float2 vertexTex;
    OutputType output;

	/*float3 vertN;
	vertN.x = patch.tex.x - 1;
	vertN.y = patch.position.y;
	vertN.z = patch.tex.y;

	float3 vertE;
	vertE.x = patch.tex.x;
	vertE.y = patch.position.y;
	vertE.z = patch.tex.y - 1;

	float3 vertS;
	vertS.x = patch.tex.x + 1;
	vertS.y = patch.position.y;
	vertS.z = patch.tex.y;

	float3 vertW;
	vertW.x = patch.tex.x;
	vertW.y = patch.position.y;
	vertW.z = patch.tex.y + 1;

	float3 vecNorth = normalize(vertN - patch.position);
	float3 vecEast = normalize(vertE - patch.position);
	float3 vecSouth = normalize(vertS - patch.position);
	float3 vecWest = normalize(vertW - patch.position);

	float3 crossNE = cross(vecEast, vecNorth);
	float3 crossSE = cross(vecSouth, vecEast);
	float3 crossSW = cross(vecWest, vecSouth);
	float3 crossNW = cross(vecNorth, vecWest);

	patch.normal = (crossSW + crossNE + crossSE + crossNW) / 4;*/
 
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

	float4 textureColour = texture0.SampleLevel(sampler0, output.tex, 0);
	float offset = textureColour.x * 20;
	vertexPosition += offset  * output.normal;

    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Send the input color into the pixel shader.
    //output.colour = patch[0].colour;
	//output.colour = textureColour;

	//output.tex = patch[0].tex;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(vertexNormal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

    return output;
}

