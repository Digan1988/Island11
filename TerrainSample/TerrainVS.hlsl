struct HSIn_Heightfield
{
	float2 origin   : ORIGIN;
	float2 size     : SIZE;
};

struct PSIn_Diffuse
{
	float4 position     : SV_Position;
	centroid float2 texcoord     : TEXCOORD0;
	centroid float3 normal       : NORMAL;
	centroid float3 positionWS   : TEXCOORD1;
	centroid float4 layerdef		: TEXCOORD2;
	centroid float4 depthmap_scaler: TEXCOORD3;
};

struct VSIn_Default
{
	float4 position : POSITION;
	float2 texcoord  : TEXCOORD;
};

struct PSIn_Quad
{
	float4 position     : SV_Position;
	float2 texcoord     : TEXCOORD0;
};

cbuffer cb: register(b0)
{

	// rendering control variables
	float		g_RenderCaustics;
	float		g_UseDynamicLOD;
	float		g_FrustumCullInHS;
	float       g_DynamicTessFactor;

	float       g_StaticTessFactor;
	float		g_TerrainBeingRendered;
	float		g_HalfSpaceCullSign;
	float		g_HalfSpaceCullPosition;

	// view/time dependent variables
	float4x4    g_ModelViewMatrix;
	float4x4    g_ModelViewProjectionMatrix;
	float4x4	g_ModelViewProjectionMatrixInv;
	float4x4    g_LightModelViewProjectionMatrix;
	float4x4    g_LightModelViewProjectionMatrixInv;
	float3      g_CameraPosition;
	float		g_SkipCausticsCalculation;
	float3      g_CameraDirection;
	int			g_MSSamples;

	float3      g_LightPosition;
	float	    g_MainBufferSizeMultiplier;
	float2      g_WaterBumpTexcoordShift;
	float2      g_ScreenSizeInv;
	
	float		g_ZNear;
	float		g_ZFar;

	// constants defining visual appearance
	float2		g_DiffuseTexcoordScale = { 130.0, 130.0 };
	float2		g_RockBumpTexcoordScale = { 10.0, 10.0 };
	float2		g_SandBumpTexcoordScale = { 3.5, 3.5 };

	float		g_RockBumpHeightScale = 3.0;
	float		g_SandBumpHeightScale = 0.5;
	float       g_TerrainSpecularIntensity = 0.5;
	float		g_WaterHeightBumpScale = 1.0f;

	float2		g_WaterMicroBumpTexcoordScale = { 225, 225 };
	float2		g_WaterBumpTexcoordScale = { 7, 7 };
	
	float3      g_WaterDeepColor = { 0.1, 0.4, 0.7 };
	float       g_WaterSpecularIntensity = 350.0;

	float3      g_WaterScatterColor = { 0.3, 0.7, 0.6 };
	float       g_WaterSpecularPower = 1000;

	float3      g_WaterSpecularColor = { 1, 1, 1 };
	float		g_FogDensity = 1.0f / 700.0f;

	float2      g_WaterColorIntensity = { 0.1, 0.2 };
	float2		g_HeightFieldOrigin = float2(0, 0);

	float3      g_AtmosphereBrightColor = { 1.0, 1.1, 1.4 };
	float		g_HeightFieldSize = 512;

	float3      g_AtmosphereDarkColor = { 0.6, 0.6, 0.7 };
	float       unuse = 0;
};

HSIn_Heightfield PassThroughVS(float4 PatchParams : PATCH_PARAMETERS)
{
	HSIn_Heightfield output;
	output.origin = PatchParams.xy;
	output.size = PatchParams.zw;
	return output;
}

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

PSIn_Diffuse SkyVS(VSIn_Default input)
{
	PSIn_Diffuse output;

	output.position = mul(input.position, g_ModelViewProjectionMatrix);
	output.positionWS = input.position.xyz;
	output.texcoord = input.texcoord;
	return output;
}

PSIn_Quad FullScreenQuadVS(uint VertexId: SV_VertexID)
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

float4 main(float4 pos : POSITION) : SV_POSITION
{
	return pos;
}