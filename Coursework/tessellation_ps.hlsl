// Tessellation pixel shader
Texture2D texture0 : register(t0);
Texture2D depthMapTextures[2] : register(t1);

SamplerState sampler0 : register(s0);
SamplerState shadowSampler : register(s1);

cbuffer LightBuffer : register(b0)
{
	float4 ambientColour;
	float4 diffuseColour[2];
	float4 lightDirection[2];
	float4 spotPosition;
};

struct InputType
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 lightViewPos[2] : TEXCOORD1;
	float4 depthPosition : TEXCOORD3;
	float3 worldPosition : TEXCOORD4;
};

float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(diffuse * intensity);
	return colour;
}

float4 main(InputType input) : SV_TARGET
{
	float depthValue[2];
	float lightDepthValue[2];
	float shadowMapBias = 0.005f;
	float4 textureColour;
	float4 colour[2];

	textureColour = texture0.Sample(sampler0, input.tex);

	float2 pTexCoord[2];

	//Spotlight calculations
	float3 lightVector = normalize(spotPosition.xyz - input.worldPosition);
	float3 lightVectorIn = -lightVector;

	float cosA = dot(lightVectorIn, lightDirection[1].xyz);
	float result = pow(max(cosA, 0), 40);

	for (int i = 0; i < 2; i++)
	{
		pTexCoord[i] = input.lightViewPos[i].xy / input.lightViewPos[i].w;
		pTexCoord[i] *= float2(0.5, -0.5);
		pTexCoord[i] += float2(0.5f, 0.5f);

		// Determine if the projected coordinates are in the 0 to 1 range.  If not don't do lighting.
		if (pTexCoord[i].x < 0.f || pTexCoord[i].x > 1.f || pTexCoord[i].y < 0.f || pTexCoord[i].y > 1.f)
		{
			return textureColour;
		}
	
		depthValue[i] = depthMapTextures[i].Sample(shadowSampler, pTexCoord[i]).r;

		lightDepthValue[i] = input.lightViewPos[i].z / input.lightViewPos[i].w;
		lightDepthValue[i] -= shadowMapBias;

		colour[i] = float4(0.f, 0.f, 0.f, 1.f);
	}

	if (lightDepthValue[0] < depthValue[0])
	{
		colour[0] = saturate(calculateLighting(-lightDirection[0].xyz, input.normal, diffuseColour[0]));
	}
		
	if (lightDepthValue[1] < depthValue[1])
	{
		colour[1] = saturate(calculateLighting(-lightDirection[1], input.normal, diffuseColour[1]) * result);
	}

	return (saturate(colour[0] + colour[1] + ambientColour)) * textureColour;
}