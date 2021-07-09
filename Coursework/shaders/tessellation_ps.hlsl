// Tessellation pixel shader
// Output colour passed to stage.
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

float4 main(InputType input) : SV_TARGET
{
	float4 textureColour;

	textureColour = texture0.Sample(sampler0, input.tex);
    return textureColour;
}