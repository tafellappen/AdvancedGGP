struct VertexToPixel
{	float2 uv				: TEXCOORD;
	float3 worldPos			: POSITION; // The world position of this vertex
};

//texture for the particles
Texture2D Texture			: register(t0);
SamplerState BasicSampler	: register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	return Texture.Sample(BasicSampler, input.uv);
}