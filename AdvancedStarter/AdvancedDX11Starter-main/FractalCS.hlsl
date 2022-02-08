// ReadWrite texture
RWTexture2D<unorm float4> outputTexture : register(u0);

cbuffer data : register(b0)
{
	float height;
	float width;

    float scale;
    float2 center;
    float2 aspectRatio;
    int maxIter;
}

//https://gamedev.stackexchange.com/questions/147890/is-there-an-hlsl-equivalent-to-glsls-map-function
//is it just me or does everything i wish had a map function built in just not have it
float MapValues(float value, float min1, float max1, float min2, float max2)
{
    // Convert the current value to a percentage
    // 0% - min1, 100% - max1
    float perc = (value - min1) / (max1 - min1);

    // Do the same operation backwards with min2 and max2
    return perc * (max2 - min2) + min2;
}

float Mandelbrot(float2 complexPlanePosition, float scale, float2 center, int maxIter)
{
    //float k = 0.0009765625;							// this is simply 1/1024, used to project 1024x1024 texture space to a 2x2 fractal space
    //double dx, dy;
    //double p, q;
    //double x, y, xnew, ynew, d = 0;					// we use double precision variables, to avoid precision limit for a bit longer while going deeper in the fractal
    //uint itn = 0;
    //dx = rect[2] - rect[0];
    //dy = rect[3] - rect[1];
    //p = rect[0] + ((int)id.x) * k * dx;
    //q = rect[1] + ((int)id.y) * k * dy;
    //x = p;
    //y = q;
    //while (itn < 255 && d < 4) {						// the essense of the fractal: in this loop we check how many steps it takes for a point to leave 2x2 fractal area
    //    xnew = x * x - y * y + p;
    //    ynew = 2 * x * y + q;
    //    x = xnew;
    //    y = ynew;
    //    d = x * x + y * y;
    //    itn++;
    //}

    // the actual mandelbrot math code is based on the GLSL at this link: http://nuclear.mutantstargoat.com/articles/sdr_fract/
    //float2 center = float2(-.0f, 0.0f);
    //int maxIter = 5000;
    //float scaleDefault = 1.0f;
    //scale += scaleDefault;

    float2 z;
    float2 c;
    //scale = 0.0009765625;
    c.x = (complexPlanePosition.x) * scale - center.x;
    c.y = (complexPlanePosition.y) * scale - center.y;

    int i;
    z = c;
    for (i = 0; i < maxIter; i++) {
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (2 * z.x * z.y) + c.y;

        if ((x * x + y * y) > 4.0)
            break;
        z.x = x;
        z.y = y;
    }
    
    return (i == maxIter ? 0.0 : float(i)) / 100.0;
}

// Specifies the number of threads in a group
[numthreads(8, 8, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{

    float2 complexPlanePosition = float2(
        MapValues(threadID.x, 0.0f, width, -aspectRatio.x, aspectRatio.x), //i think these two are what i want, but the hardcoded numbers are a known working for now
        MapValues(threadID.y, 0.0f, height, -aspectRatio.y, aspectRatio.y)
        //MapValues(threadID.x, 0.0f, width, -2.0f, 2.0f),
        //MapValues(threadID.y, 0.0f, height, -1.0f, 1.0f)
        );
    /*MapValues(threadID.x, 0.0f, width, -aspectRatio.x, aspectRatio.x),
        MapValues(threadID.y, 0.0f, height, -aspectRatio.y, aspectRatio.y)*/
	float iterations = Mandelbrot(complexPlanePosition, scale, center, maxIter);

	// Store in the texture at [x,y]
	//outputTexture[threadID.xy] = float4(0, 0, 1, 1);
	outputTexture[threadID.xy] = float4(iterations, iterations, iterations, 1);
}