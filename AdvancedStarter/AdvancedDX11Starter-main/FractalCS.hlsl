// ReadWrite texture
RWTexture2D<unorm float4> outputTexture : register(u0);

cbuffer data : register(b0)
{
	//float realExtents;
	//float complexExtents;
}

float Mandelbrot() 
{
	return float(1.0);
}

// Specifies the number of threads in a group
[numthreads(8, 8, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
	return float4(1, 0, 1, 1);
	float iterations = Mandelbrot();

	// Store in the texture at [x,y]
	outputTexture[threadID.xy] = float4(iterations, iterations, iterations, 1);
}