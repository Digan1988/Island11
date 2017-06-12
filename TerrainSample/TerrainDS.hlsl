struct PSIn_Diffuse
{
	float4 position     : SV_Position;
	centroid float2 texcoord     : TEXCOORD0;
	centroid float3 normal       : NORMAL;
	centroid float3 positionWS   : TEXCOORD1;
	centroid float4 layerdef		: TEXCOORD2;
	centroid float4 depthmap_scaler: TEXCOORD3;
};

struct PatchData
{
	float Edges[4]  : SV_TessFactor;
	float Inside[2]	: SV_InsideTessFactor;

	float2 origin   : ORIGIN;
	float2 size     : SIZE;
};

struct DUMMY
{
	float Dummmy : DUMMY;
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
	float		g_WaterHeightBumpScale = 0.0f;

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
Texture2D g_LayerdefTexture : register(t1);
Texture2D g_SandBumpTexture : register(t2);
Texture2D g_RockBumpTexture : register(t3);
Texture2D g_WaterNormalMapTexture : register(t4);
Texture2D g_DepthMapTexture : register(t5);
Texture2D g_WaterBumpTexture : register(t6);

SamplerState SamplerLinearWrap :register(s0);

// calculating tessellation factor. It is either constant or hyperbolic depending on g_UseDynamicLOD switch
float CalculateTessellationFactor(float distance)
{
	//tmp
	float g_StaticTessFactor = 1.0f;
	float g_DynamicTessFactor = 1.0f;
	float g_UseDynamicLOD = 1.0f;
	return lerp(g_StaticTessFactor, g_DynamicTessFactor*(1 / (0.015*distance)), g_UseDynamicLOD);
}

// to avoid vertex swimming while tessellation varies, one can use mipmapping for displacement maps
// it's not always the best choice, but it effificiently suppresses high frequencies at zero cost
float CalculateMIPLevelForDisplacementTextures(float distance)
{
	return log2(128 / CalculateTessellationFactor(distance));
}

// calculating water refraction caustics intensity
float CalculateWaterCausticIntensity(float3 worldpos)
{
	//tmp
	//float3  g_CameraPosition = float3(365.000031, 3.000000, 166.000031);
	float3  g_LightPosition = float3(-10000.0f, 6500.0f, 10000.0f);

	float distance_to_camera = length(g_CameraPosition - worldpos);

	float2 refraction_disturbance;
	float3 n;
	float m = 0.2;
	float cc = 0;
	float k = 0.15;
	float water_depth = 0.5 - worldpos.y;

	float3 pixel_to_light_vector = normalize(g_LightPosition - worldpos);

		worldpos.xz -= worldpos.y*pixel_to_light_vector.xz;
	float3 pixel_to_water_surface_vector = pixel_to_light_vector*water_depth;
		float3 refracted_pixel_to_light_vector;

	// tracing approximately refracted rays back to light
	for (float i = -3; i <= 3; i += 1)
		for (float j = -3; j <= 3; j += 1)
		{
			n = 2.0f*g_WaterNormalMapTexture.SampleLevel(SamplerLinearWrap, (worldpos.xz - g_CameraPosition.xz - float2(200.0, 200.0) + float2(i*k, j*k)*m*water_depth) / 400.0, 0).rgb - float3(1.0f, 1.0f, 1.0f);
			refracted_pixel_to_light_vector = m*(pixel_to_water_surface_vector + float3(i*k, 0, j*k)) - 0.5*float3(n.x, 0, n.z);
			cc += 0.05*max(0, pow(max(0, dot(normalize(refracted_pixel_to_light_vector), normalize(pixel_to_light_vector))), 500.0f));
		}
	return cc;
}

[domain("quad")]
PSIn_Diffuse HeightFieldPatchDS(PatchData input,
	float2 uv : SV_DomainLocation,
	OutputPatch<DUMMY, 1> inputPatch)
{
	PSIn_Diffuse output;

	//tmp
	float g_HeightFieldSize = 512;
	//float3  g_CameraPosition = float3(365.000031, 3.000000, 166.000031);
	float2	g_SandBumpTexcoordScale = { 3.5, 3.5 };
	float	g_SandBumpHeightScale = 0.5;
	float2	g_RockBumpTexcoordScale = { 10.0, 10.0 };
	float	g_RockBumpHeightScale = 3.0;
	float	g_SkipCausticsCalculation = 0.0f;
	float	g_RenderCaustics = 1.0f;
	float	g_WaterHeightBumpScale = 1.0f;
	float2	g_DiffuseTexcoordScale = { 130.0, 130.0 };

	float2 texcoord0to1 = (input.origin + uv * input.size) / g_HeightFieldSize;

	texcoord0to1.y = 1 - texcoord0to1.y;

	// fetching base heightmap,normal and moving vertices along y axis
	float4 base_texvalue = g_HeightfieldTexture.SampleLevel(SamplerLinearWrap, texcoord0to1, 0);
	float3 base_normal = base_texvalue.xyz;
	base_normal.z = -base_normal.z;

	float3 vertexPosition;
	vertexPosition.xz = input.origin + uv * input.size;
	vertexPosition.y = base_texvalue.w;

	// calculating MIP level for detail texture fetches
	float distance_to_camera = length(g_CameraPosition - vertexPosition);
	float detailmap_miplevel = CalculateMIPLevelForDisplacementTextures(distance_to_camera);//log2(1+distance_to_camera*3000/(g_HeightFieldSize*g_TessFactor));

	// fetching layer definition texture
	float4 layerdef = g_LayerdefTexture.SampleLevel(SamplerLinearWrap, texcoord0to1, 0);

		// default detail texture
		float4 detail_texvalue = g_SandBumpTexture.SampleLevel(SamplerLinearWrap, texcoord0to1*g_SandBumpTexcoordScale, detailmap_miplevel).rbga;
		float3 detail_normal = normalize(2 * detail_texvalue.xyz - float3(1, 0, 1));
		float detail_height = (detail_texvalue.w - 0.5)*g_SandBumpHeightScale;

	// rock detail texture
	detail_texvalue = g_RockBumpTexture.SampleLevel(SamplerLinearWrap, texcoord0to1*g_RockBumpTexcoordScale, detailmap_miplevel).rbga;
	detail_normal = lerp(detail_normal, normalize(2 * detail_texvalue.xyz - float3(1, 1.4, 1)), layerdef.w);
	detail_height = lerp(detail_height, (detail_texvalue.w - 0.5)*g_RockBumpHeightScale, layerdef.w);

	// moving vertices by detail height along base normal
	vertexPosition += base_normal*detail_height;

	//calculating base normal rotation matrix
	float3x3 normal_rotation_matrix;
	normal_rotation_matrix[1] = base_normal;
	normal_rotation_matrix[2] = normalize(cross(float3(-1.0, 0.0, 0.0), normal_rotation_matrix[1]));
	normal_rotation_matrix[0] = normalize(cross(normal_rotation_matrix[2], normal_rotation_matrix[1]));

	//applying base rotation matrix to detail normal
	float3 detail_normal_rotated = mul(detail_normal, normal_rotation_matrix);

		//adding refraction caustics
		float cc = 0;

	if ((g_SkipCausticsCalculation == 0) && (g_RenderCaustics>0)) // doing it only for main
	{
		cc = CalculateWaterCausticIntensity(vertexPosition.xyz);
	}

	// fading caustics out at distance
	cc *= (200.0 / (200.0 + distance_to_camera));

	// fading caustics out as we're getting closer to water surface
	cc *= min(1, max(0, -g_WaterHeightBumpScale - vertexPosition.y));


	// writing output params
	//matrix g_ModelViewProjectionMatrix = viewMatrix * projectionMatrix;

	output.position = mul(float4(vertexPosition, 1.0), g_ModelViewProjectionMatrix);
	output.texcoord = texcoord0to1*g_DiffuseTexcoordScale;
	output.normal = detail_normal_rotated;
	output.positionWS = vertexPosition;
	output.layerdef = layerdef;
	output.depthmap_scaler = float4(1.0, 1.0, detail_height, cc);

	return output;
}

// constructing the displacement amount and normal for water surface geometry
float4 CombineWaterNormal(float3 world_position)
{
	float4 water_normal = float4(0.0, 4.0, 0.0, 0.0);
		float water_miplevel;
	float distance_to_camera;
	float4 texvalue;
	float texcoord_scale = 1.0;
	float height_disturbance_scale = 1.0;
	float normal_disturbance_scale = 1.0;
	float2 tc;
	float2 variance = { 1.0, 1.0 };

	// calculating MIP level for water texture fetches
	distance_to_camera = length(g_CameraPosition - world_position);
	water_miplevel = CalculateMIPLevelForDisplacementTextures(distance_to_camera) / 2.0 - 2.0;
	tc = (world_position.xz*g_WaterBumpTexcoordScale / g_HeightFieldSize);

	// fetching water heightmap
	for (float i = 0; i<5; i++)
	{
		texvalue = g_WaterBumpTexture.SampleLevel(SamplerLinearWrap, tc*texcoord_scale + g_WaterBumpTexcoordShift*0.03*variance, water_miplevel).rbga;
		variance.x *= -1.0;
		water_normal.xz += (2 * texvalue.xz - float2(1.0, 1.0))*normal_disturbance_scale;
		water_normal.w += (texvalue.w - 0.5)*height_disturbance_scale;
		texcoord_scale *= 1.4;
		height_disturbance_scale *= 0.65;
		normal_disturbance_scale *= 0.65;
	}
	water_normal.w *= g_WaterHeightBumpScale;
	return float4(normalize(water_normal.xyz), water_normal.w);
}

[domain("quad")]
PSIn_Diffuse WaterPatchDS(PatchData input,
	float2 uv : SV_DomainLocation,
	OutputPatch<DUMMY, 1> inputPatch)
{
	PSIn_Diffuse output;
	float3 vertexPosition;
	float2 texcoord0to1 = (input.origin + uv * input.size) / g_HeightFieldSize;
		float4 water_normal;
	float4 depthmap_scaler;

	// getting rough estimate of water depth from depth map texture 
	depthmap_scaler = g_DepthMapTexture.SampleLevel(SamplerLinearWrap, float2(texcoord0to1.x, 1 - texcoord0to1.y), 0);

	// calculating water surface geometry position and normal
	vertexPosition.xz = input.origin + uv * input.size;
	vertexPosition.y = -g_WaterHeightBumpScale / 2;
	water_normal = CombineWaterNormal(vertexPosition.xyz);

	// fading out displacement and normal disturbance near shores by 60%
	water_normal.xyz = lerp(float3(0, 1, 0), normalize(water_normal.xyz), 0.4 + 0.6*depthmap_scaler.g);
	vertexPosition.y += water_normal.w*g_WaterHeightBumpScale*(0.4 + 0.6*depthmap_scaler.g);
	vertexPosition.xz -= (water_normal.xz)*0.5*(0.4 + 0.6*depthmap_scaler.g);

	// writing output params
	output.position = mul(float4(vertexPosition, 1.0), g_ModelViewProjectionMatrix);
	output.texcoord = texcoord0to1*g_WaterMicroBumpTexcoordScale + g_WaterBumpTexcoordShift*0.07;
	output.normal = normalize(water_normal.xyz);
	output.depthmap_scaler = depthmap_scaler;
	output.positionWS = vertexPosition;
	return output;
}

[domain("quad")]
PSIn_Diffuse main(PatchData input, float2 uv : SV_DomainLocation, OutputPatch<DUMMY, 1> inputPatch)
{
	PSIn_Diffuse output;
	float3 vertexPosition;
	float2 texcoord0to1 = (input.origin + uv * input.size) / g_HeightFieldSize;
		float4 water_normal;
	float4 depthmap_scaler;

	// getting rough estimate of water depth from depth map texture 
	depthmap_scaler = g_DepthMapTexture.SampleLevel(SamplerLinearWrap, float2(texcoord0to1.x, 1 - texcoord0to1.y), 0);

	// calculating water surface geometry position and normal
	vertexPosition.xz = input.origin + uv * input.size;
	vertexPosition.y = -g_WaterHeightBumpScale / 2;
	water_normal = CombineWaterNormal(vertexPosition.xyz);

	// fading out displacement and normal disturbance near shores by 60%
	water_normal.xyz = lerp(float3(0, 1, 0), normalize(water_normal.xyz), 0.4 + 0.6*depthmap_scaler.g);
	vertexPosition.y += water_normal.w*g_WaterHeightBumpScale*(0.4 + 0.6*depthmap_scaler.g);
	vertexPosition.xz -= (water_normal.xz)*0.5*(0.4 + 0.6*depthmap_scaler.g);

	// writing output params
	output.position = mul(float4(vertexPosition, 1.0), g_ModelViewProjectionMatrix);
	output.texcoord = texcoord0to1*g_WaterMicroBumpTexcoordScale + g_WaterBumpTexcoordShift*0.07;
	output.normal = normalize(water_normal.xyz);
	output.depthmap_scaler = depthmap_scaler;
	output.positionWS = vertexPosition;
	return output;
}