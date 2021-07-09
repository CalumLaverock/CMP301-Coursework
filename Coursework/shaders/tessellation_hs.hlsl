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

	float max_tes = 64;
	float distance;

	for (int i = 0; i < 4; i++)
	{
		distance += sqrt(pow(inputPatch[i].position.x - cam_pos.x, 2) + pow(inputPatch[i].position.y - cam_pos.y, 2) + pow(inputPatch[i].position.z - cam_pos.z, 2));
	}

	float tes_factor = max(max_tes - distance, 1);

    // Set the tessellation factors for the three edges of the triangle.
	output.edges[0] = 4;
	output.edges[1] = 4;
	output.edges[2] = 4;
	output.edges[3] = 4;

    // Set the tessellation factor for tessallating inside the triangle.
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