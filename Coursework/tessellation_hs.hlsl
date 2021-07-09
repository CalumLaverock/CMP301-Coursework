// Tessellation Hull Shader
// Prepares control points for tessellation
cbuffer TesselationBuffer : register(b0)
{
	float4 cam_pos;
};

struct InputType
{
    float3 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct OutputType
{
    float3 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID)
{    
    ConstantOutputType output;

	const int max_tes = 64;
	float distance;
	float edgeDistances[4];
	int edgeFactors[4];
	float3 edges[4];

	for (int i = 0; i < 4; i++)
	{
		//Add together the distances from the camera to each corner
		distance += sqrt(pow(inputPatch[i].position.x - cam_pos.x, 2) + pow(inputPatch[i].position.y - cam_pos.y, 2) + pow(inputPatch[i].position.z - cam_pos.z, 2));
	}

	//Find the average distance from the camera to the patch
	distance /= 4;

	//Find the middle point of each edge of the patch
	edges[0] = lerp(inputPatch[0].position, inputPatch[1].position, 0.5);
	edges[1] = lerp(inputPatch[1].position, inputPatch[2].position, 0.5);
	edges[2] = lerp(inputPatch[2].position, inputPatch[3].position, 0.5);
	edges[3] = lerp(inputPatch[3].position, inputPatch[0].position, 0.5);

	for (int i = 0; i < 4; i++)
	{
		//Calculate the distance from the camera to the centre of each edge
		edgeDistances[i] = sqrt(pow(edges[i].x - cam_pos.x, 2) + pow(edges[i].y - cam_pos.y, 2) + pow(edges[i].z - cam_pos.z, 2));

		//Limit the distance from the camera that an edge will tessellate
		edgeDistances[i] *= 2;

		//Make sure the tessellation factor for each edge is never less than 1
		edgeFactors[i] = max(max_tes - edgeDistances[i], 1);
	}

	//Limit the distance from the camera that the centre of a patch will tessellate
	distance *= 2;

	//Make sure the tessellation factor for the centre of the patch is never less than 1
	int tes_factor = max(max_tes - distance, 1);

    // Set the tessellation factors for the four edges of the quads.
	output.edges[0] = edgeFactors[0];
	output.edges[1] = edgeFactors[3];
	output.edges[2] = edgeFactors[2];
	output.edges[3] = edgeFactors[1];

    // Set the tessellation factor for tessallating inside the quads.
	output.inside[0] = tes_factor;
	output.inside[1] = tes_factor;

    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;

    // Set the input colour as the output colour.
	output.tex = patch[pointId].tex;
	output.normal = patch[pointId].normal;

    return output;
}