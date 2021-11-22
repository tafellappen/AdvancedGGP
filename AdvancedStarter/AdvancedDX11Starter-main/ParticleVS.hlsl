cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float currentTime;
};

struct Particle
{
	float EmitTime;
	float3 StartPosition;
};

// structured buffer of particle data
StructuredBuffer<Particle> ParticleData : register(t0);

struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

struct VertexToPixel
{
	float2 uv				: TEXCOORD;
	float4 position			: POSITION;
};

VertexToPixel main(uint id : SV_VertexID)
{
	VertexToPixel output;



	////test stuff so i can try to get just the cpp working first
	//output.worldPos = mul(world, float4(input.position, 1.0f)).xyz;
	//output.uv = input.uv;

	//use vertex's id to calc the particle index and corner index
	uint particleID = id / 4; // 4 verticies to one particle
	uint cornerID = id % 4; // 0,1,2,3 = the corner of the particle "quad"


	//retreive the proper particle from the structured buffer
	//grab one particle and calculate its age
	Particle p = ParticleData.Load(particleID);

	//simulation:
	//calculate the age of the particle (in seconds, and/or as percent of lifetime)
	float age = currentTime - p.EmitTime;
	//use age along with other particle data to calculate a final psition
	float3 pos = p.StartPosition;
	pos += float3(0.5f, 0, 0) * age;
	//pos = accel * age * age / 2.0f + p.StartVelocity * age + p.StartPosition;

	// Offsets for the 4 corners of a quad - we'll only
	// use one for each vertex, but which one depends
	// on the cornerID above.
	float2 offsets[4];
	offsets[0] = float2(-1.0f, +1.0f);  // TL
	offsets[1] = float2(+1.0f, +1.0f);  // TR
	offsets[2] = float2(+1.0f, -1.0f);  // BR
	offsets[3] = float2(-1.0f, -1.0f);  // BL
	
	//perform billboarding
	// Offset the position based on the camera's right and up vectors
	pos += float3(view._11, view._12, view._13) * offsets[cornerID].x; // RIGHT
	pos += float3(view._21, view._22, view._23) * offsets[cornerID].y; // UP

	//handle camera matrix transformations
	matrix viewProj = mul(projection, view);
	output.position = mul(viewProj, float4(pos, 1.0f));

	//determine proper UV index for this vertex
	// UVs for the 4 corners of a quad - again, only
	// using one for each vertex, but which one depends
	// on the cornerID above.
	float2 uvs[4];
	uvs[0] = float2(0, 0); // TL
	uvs[1] = float2(1, 0); // TR
	uvs[2] = float2(1, 1); // BR
	uvs[3] = float2(0, 1); // BL
	output.uv = uvs[cornerID];



	return output;
}