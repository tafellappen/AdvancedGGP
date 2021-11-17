

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};


Texture2D SceneColors	: register(t0);
//Texture2D SceneColorsNoAmbient	: register(t0);
//Texture2D Ambient				: register(t1);
SamplerState BasicSampler		: register(s0);


float4 main(VertexToPixel input) : SV_TARGET
{
	// Sample all three
	float3 sceneColors = SceneColors.Sample(BasicSampler, input.uv).rgb;
	//float3 ambient = Ambient.Sample(BasicSampler, input.uv).rgb;

	// Final combine
	return float4(pow(sceneColors, 1.0f / 2.2f), 1);
}