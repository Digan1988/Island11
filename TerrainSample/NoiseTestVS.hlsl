struct PSIn_Quad
{
	float4 position     : SV_Position;
	float2 texcoord     : TEXCOORD0;
};

PSIn_Quad WaterNormalmapCombineVS(uint VertexId: SV_VertexID)
{
	PSIn_Quad output;

	const float2 QuadVertices[4] =
	{
		{ -1.0, -1.0 },
		{ 1.0, -1.0 },
		{ -1.0, 1.0 },
		{ 1.0, 1.0 }
	};

	const float2 QuadTexCoordinates[4] =
	{
		{ 0.0, 1.0 },
		{ 1.0, 1.0 },
		{ 0.0, 0.0 },
		{ 1.0, 0.0 }
	};

	output.position = float4(QuadVertices[VertexId], 0, 1);
	output.texcoord = QuadTexCoordinates[VertexId];

	return output;
}

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}