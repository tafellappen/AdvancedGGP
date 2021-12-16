////RWTexture2D<unorm float4> outputTexture : register(u0);
//
//// Defines the input to this pixel shader
//// - Should match the output of our corresponding vertex shader
//struct VertexToPixel
//{
//	float4 screenPosition	: SV_POSITION;
//	float2 uv				: TEXCOORD;
//	float3 normal			: NORMAL;
//	float3 tangent			: TANGENT;
//	float3 worldPos			: POSITION; // The world position of this PIXEL
//};
//
//SamplerState BasicSampler		: register(s0);
//
//float4 main(VertexToPixel input) : SV_TARGET
//{
//	return float4(outputTexture[input.screenPosition.xy]);
//}
//
//

cbuffer perFrame : register(b0)
{
	float4 Color2;
}
cbuffer perMaterial : register(b1)
{
	float4 Color;
}

struct VertexToPixel
{
	float4 screenPosition		: SV_POSITION;
	float2 uv           : TEXCOORD0;
	float3 normal		: NORMAL;
};


Texture2D Pixels			: register(t0);
SamplerState BasicSampler	: register(s0);



float4 main(VertexToPixel input) : SV_TARGET
{
	//return float4(1, 0, 0, 1);
	return Pixels.Sample(BasicSampler, input.uv);
}