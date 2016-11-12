struct DUMMY
{
	float Dummmy : DUMMY;
};

struct HSIn_Heightfield
{
	float2 origin   : ORIGIN;
	float2 size     : SIZE;
};

struct PatchData
{
	float Edges[4]  : SV_TessFactor;
	float Inside[2]	: SV_InsideTessFactor;

	float2 origin   : ORIGIN;
	float2 size     : SIZE;
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

Texture2D g_HeightfieldTexture : register(t0);

SamplerState SamplerLinearWrap :register(s0);

// calculating tessellation factor. It is either constant or hyperbolic depending on g_UseDynamicLOD switch
float CalculateTessellationFactor(float distance)
{
	//tmp
	float g_StaticTessFactor = 1.0f;
	float g_DynamicTessFactor = 5.0f;
	float g_UseDynamicLOD = 1.0f;
	return lerp(g_StaticTessFactor, g_DynamicTessFactor*(1 / (0.015*distance)), g_UseDynamicLOD);
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(1)]
[patchconstantfunc("PatchConstantHS")]
DUMMY PatchHS(InputPatch<HSIn_Heightfield, 1> inputPatch)
{
	return (DUMMY)0;
}

PatchData PatchConstantHS(InputPatch<HSIn_Heightfield, 1> inputPatch)
{
	PatchData output;

	float in_frustum = 0;

	output.origin = inputPatch[0].origin;
	output.size = inputPatch[0].size;

	//tmp
	float g_HeightFieldSize = 512;
	float g_TerrainBeingRendered = 1.0f;
	//float3  g_CameraPosition = float3(365.000031, 3.000000, 166.000031);
	//float3  g_CameraDirection = normalize(float3(364.651215, 2.860487, 166.926758) - g_CameraPosition);
	float g_FrustumCullInHS = 1.0f;

	float2 texcoord0to1 = (inputPatch[0].origin + inputPatch[0].size / 2.0) / g_HeightFieldSize;
		texcoord0to1.y = 1 - texcoord0to1.y;

	// conservative frustum culling
	float3 patch_center = float3(inputPatch[0].origin.x + inputPatch[0].size.x*0.5, g_TerrainBeingRendered*g_HeightfieldTexture.SampleLevel(SamplerLinearWrap, texcoord0to1, 0).w, inputPatch[0].origin.y + inputPatch[0].size.y*0.5);
		float3 camera_to_patch_vector = patch_center - g_CameraPosition;
		float3 patch_to_camera_direction_vector = g_CameraDirection*dot(camera_to_patch_vector, g_CameraDirection) - camera_to_patch_vector;
		float3 patch_center_realigned = patch_center + normalize(patch_to_camera_direction_vector)*min(2 * inputPatch[0].size.x, length(patch_to_camera_direction_vector));
		float4 patch_screenspace_center = mul(float4(patch_center_realigned, 1.0), g_ModelViewProjectionMatrix);

		if (((patch_screenspace_center.x / patch_screenspace_center.w>-1.0) && (patch_screenspace_center.x / patch_screenspace_center.w<1.0)
			&& (patch_screenspace_center.y / patch_screenspace_center.w>-1.0) && (patch_screenspace_center.y / patch_screenspace_center.w<1.0)
			&& (patch_screenspace_center.w>0)) || (length(patch_center - g_CameraPosition)<2 * inputPatch[0].size.x))
		{
			in_frustum = 1;
		}

	if ((in_frustum) || (g_FrustumCullInHS == 0))
	{
		float inside_tessellation_factor = 0;

		float distance_to_camera = length(g_CameraPosition.xz - inputPatch[0].origin - float2(0, inputPatch[0].size.y*0.5));
		float tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[0] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;


		distance_to_camera = length(g_CameraPosition.xz - inputPatch[0].origin - float2(inputPatch[0].size.x*0.5, 0));
		tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[1] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;

		distance_to_camera = length(g_CameraPosition.xz - inputPatch[0].origin - float2(inputPatch[0].size.x, inputPatch[0].size.y*0.5));
		tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[2] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;

		distance_to_camera = length(g_CameraPosition.xz - inputPatch[0].origin - float2(inputPatch[0].size.x*0.5, inputPatch[0].size.y));
		tesselation_factor = CalculateTessellationFactor(distance_to_camera);
		output.Edges[3] = tesselation_factor;
		inside_tessellation_factor += tesselation_factor;
		output.Inside[0] = output.Inside[1] = inside_tessellation_factor*0.25;
	}
	else
	{
		output.Edges[0] = -1;
		output.Edges[1] = -1;
		output.Edges[2] = -1;
		output.Edges[3] = -1;
		output.Inside[0] = -1;
		output.Inside[1] = -1;
	}

	return output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(1)]
[patchconstantfunc("PatchConstantHS")]
DUMMY main(InputPatch<HSIn_Heightfield, 1> inputPatch)
{
	return (DUMMY)0;
}