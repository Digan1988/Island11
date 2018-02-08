struct PSIn_Diffuse
{
	float4 position     : SV_Position;
	centroid float2 texcoord     : TEXCOORD0;
	centroid float3 normal       : NORMAL;
	centroid float3 positionWS   : TEXCOORD1;
	centroid float4 layerdef		: TEXCOORD2;
	centroid float4 depthmap_scaler: TEXCOORD3;
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

Texture2D g_SandMicroBumpTexture : register(t0);
Texture2D g_RockMicroBumpTexture : register(t1);
Texture2D g_SlopeDiffuseTexture : register(t2);
Texture2D g_SandDiffuseTexture : register(t3);
Texture2D g_RockDiffuseTexture : register(t4);
Texture2D g_GrassDiffuseTexture : register(t5);
Texture2D g_DepthTexture : register(t6);
Texture2D g_WaterBumpTexture : register(t7);
Texture2D g_SkyTexture : register(t8);
Texture2D g_RefractionDepthTextureResolved : register(t9);
Texture2D g_ReflectionTexture : register(t10);
Texture2D g_RefractionTexture : register(t11);
Texture2D g_MainTexture : register(t12);
Texture2DMS<float, 1> g_RefractionDepthTextureMS1 : register(t13);
Texture2DMS<float, 2> g_RefractionDepthTextureMS2 : register(t14);
Texture2DMS<float, 4> g_RefractionDepthTextureMS4 : register(t15);

SamplerState SamplerAnisotropicWrap:register(s0);

SamplerComparisonState SamplerDepthAnisotropic:register(s1);

SamplerState SamplerLinearWrap:register(s2);

SamplerState SamplerLinearClamp:register(s3);

SamplerState SamplerPointClamp:register(s4);

// primitive simulation of non-uniform atmospheric fog
float3 CalculateFogColor(float3 pixel_to_light_vector, float3 pixel_to_eye_vector)
{
	return lerp(g_AtmosphereDarkColor, g_AtmosphereBrightColor, 0.5*dot(pixel_to_light_vector, -pixel_to_eye_vector) + 0.5);
}

float4 HeightFieldPatchPS(PSIn_Diffuse input) : SV_Target
{
	// culling halfspace if needed
	clip(g_HalfSpaceCullSign*(input.positionWS.y - g_HalfSpaceCullPosition));

	// fetching default microbump normal
	float3 microbump_normal = normalize(2 * g_SandMicroBumpTexture.Sample(SamplerAnisotropicWrap, input.texcoord).rbg - float3 (1.0, 1.0, 1.0));
	microbump_normal = normalize(lerp(microbump_normal, 2 * g_RockMicroBumpTexture.Sample(SamplerAnisotropicWrap, input.texcoord).rbg - float3 (1.0, 1.0, 1.0), input.layerdef.w));

	//calculating base normal rotation matrix
	float3x3 normal_rotation_matrix;
	normal_rotation_matrix[1] = input.normal;
	normal_rotation_matrix[2] = normalize(cross(float3(-1.0, 0.0, 0.0), normal_rotation_matrix[1]));
	normal_rotation_matrix[0] = normalize(cross(normal_rotation_matrix[2], normal_rotation_matrix[1]));
	microbump_normal = mul(microbump_normal, normal_rotation_matrix);

	// getting diffuse color
	float4 color = g_SlopeDiffuseTexture.Sample(SamplerAnisotropicWrap, input.texcoord);
	color = lerp(color, g_SandDiffuseTexture.Sample(SamplerAnisotropicWrap, input.texcoord), input.layerdef.g*input.layerdef.g);
	color = lerp(color, g_RockDiffuseTexture.Sample(SamplerAnisotropicWrap, input.texcoord), input.layerdef.w*input.layerdef.w);
	color = lerp(color, g_GrassDiffuseTexture.Sample(SamplerAnisotropicWrap, input.texcoord), input.layerdef.b);

	// adding per-vertex lighting defined by displacement of vertex 
	color *= 0.5 + 0.5*min(1.0, max(0.0, input.depthmap_scaler.b / 3.0f + 0.5f));

	// calculating pixel position in light view space
	float4 positionLS = mul(float4(input.positionWS, 1), g_LightModelViewProjectionMatrix);
		positionLS.xyz /= positionLS.w;
	positionLS.x = (positionLS.x + 1)*0.5;
	positionLS.y = (1 - positionLS.y)*0.5;


	// fetching shadowmap and shading
	float dsf = 0.75f / 4096.0f;
	float shadow_factor = 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy, positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(dsf, dsf), positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(-dsf, dsf), positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(dsf, -dsf), positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(-dsf, -dsf), positionLS.z* 0.995f).r;

	float3 pixel_to_light_vector = normalize(g_LightPosition - input.positionWS);

	color.rgb *= max(0, dot(pixel_to_light_vector, microbump_normal))*shadow_factor + 0.2;

	// adding light from the sky
	color.rgb += (0.0 + 0.2*max(0, (dot(float3(0, 1, 0), microbump_normal))))*float3(0.2, 0.2, 0.3);

	// making all a bit brighter, simultaneously pretending the wet surface is darker than normal;
	color.rgb *= 0.5 + 0.8*max(0, min(1, input.positionWS.y*0.5 + 0.5));

	// applying refraction caustics
	color.rgb *= (1.0 + max(0, 0.4 + 0.6*dot(pixel_to_light_vector, microbump_normal))*input.depthmap_scaler.a*(0.4 + 0.6*shadow_factor));

	float3 pixel_to_eye_vector = normalize(g_CameraPosition - input.positionWS);

	// applying fog
	color.rgb = lerp(CalculateFogColor(pixel_to_light_vector, pixel_to_eye_vector).rgb, color.rgb, min(1, exp(-length(g_CameraPosition - input.positionWS)*g_FogDensity)));
	color.a = length(g_CameraPosition - input.positionWS);

	return color;
}

float4 ColorPS(uniform float4 color) : SV_Target
{
	return color;
}

// constructing water surface normal for water refraction caustics
float3 CombineSimplifiedWaterNormal(float3 world_position, float mip_level)
{
	float3 water_normal = float3(0.0, 4.0, 0.0);

	float water_miplevel;
	float distance_to_camera;
	float4 texvalue;
	float texcoord_scale = 1.0;
	float normal_disturbance_scale = 1.0;
	float2 variance = { 1.0, 1.0 };

	//
	float2	g_WaterBumpTexcoordScale = { 7, 7 };
	float	g_HeightFieldSize = 512;
	//float2  g_WaterBumpTexcoordShift = float2(0.000914, 0.000457);

	float2 tc = (world_position.xz*g_WaterBumpTexcoordScale / g_HeightFieldSize);

	// need more high frequensy details for caustics, so summing more "octaves"
	for (float i = 0; i<8; i++)
	{
		texvalue = g_WaterBumpTexture.SampleLevel(SamplerLinearWrap, tc*texcoord_scale + g_WaterBumpTexcoordShift*0.03*variance, mip_level/*+i*/).rbga;
		variance.x *= -1.0;
		water_normal.xz += (2 * texvalue.xz - float2(1, 1))*normal_disturbance_scale;
		texcoord_scale *= 1.4;
		normal_disturbance_scale *= 0.85;
	}

	return normalize(water_normal);
}

float4 WaterNormalmapCombinePS(PSIn_Quad input) : SV_Target
{
	float4 color = float4(1,0,0,1);

	color.rgb = (CombineSimplifiedWaterNormal(g_CameraPosition + float3(input.texcoord.x*400.0f - 200.0f, 0, input.texcoord.y*400.0f - 200.0f), 0).rgb + float3(1.0f, 1.0f, 1.0f))*0.5f;
	color.a = 0;
	return color;
}

float4 SkyPS(PSIn_Diffuse input) : SV_Target
{
	float4 color;
	float3 acolor;
	float3 pixel_to_light_vector = normalize(g_LightPosition - input.positionWS);
	float3 pixel_to_eye_vector = normalize(g_CameraPosition - input.positionWS);

	color = g_SkyTexture.Sample(SamplerLinearWrap, float2(input.texcoord.x, pow(input.texcoord.y, 2)));
	acolor = CalculateFogColor(pixel_to_light_vector, pixel_to_eye_vector);
	color.rgb = lerp(color.rgb, acolor, pow(saturate(input.texcoord.y), 10));
	color.a = 1;
	return color;
}

float RefractionDepthManualResolvePS1(PSIn_Quad input) : SV_Target
{
	return g_RefractionDepthTextureMS1.Load(input.position.xy, 0, int2(0, 0)).r;
}

float RefractionDepthManualResolvePS2(PSIn_Quad input) : SV_Target
{
	return g_RefractionDepthTextureMS2.Load(input.position.xy, 0, int2(0, 0)).r;
}

float RefractionDepthManualResolvePS4(PSIn_Quad input) : SV_Target
{
	return g_RefractionDepthTextureMS4.Load(input.position.xy, 0, int2(0, 0)).r;
}

float GetRefractionDepth(float2 position)
{
	return g_RefractionDepthTextureResolved.SampleLevel(SamplerLinearClamp, position, 0).r;
}

float GetConservativeRefractionDepth(float2 position)
{
	float result = g_RefractionDepthTextureResolved.SampleLevel(SamplerPointClamp, position + 2.0*float2(g_ScreenSizeInv.x, g_ScreenSizeInv.y), 0).r;
	result = min(result, g_RefractionDepthTextureResolved.SampleLevel(SamplerPointClamp, position + 2.0*float2(g_ScreenSizeInv.x, -g_ScreenSizeInv.y), 0).r);
	result = min(result, g_RefractionDepthTextureResolved.SampleLevel(SamplerPointClamp, position + 2.0*float2(-g_ScreenSizeInv.x, g_ScreenSizeInv.y), 0).r);
	result = min(result, g_RefractionDepthTextureResolved.SampleLevel(SamplerPointClamp, position + 2.0*float2(-g_ScreenSizeInv.x, -g_ScreenSizeInv.y), 0).r);
	return result;
}

float4 WaterPatchPS(PSIn_Diffuse input) : SV_Target
{
	// calculating pixel position in light space
	float4 positionLS = mul(float4(input.positionWS, 1), g_LightModelViewProjectionMatrix);
		positionLS.xyz /= positionLS.w;
	positionLS.x = (positionLS.x + 1)*0.5;
	positionLS.y = (1 - positionLS.y)*0.5;

	// calculating shadow multiplier to be applied to diffuse/scatter/specular light components
	float dsf = 1.0f / 4096.0f;
	float shadow_factor = 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy, positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(dsf, dsf), positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(-dsf, dsf), positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(dsf, -dsf), positionLS.z* 0.995f).r;
	shadow_factor += 0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic, positionLS.xy + float2(-dsf, -dsf), positionLS.z* 0.995f).r;

	// calculating base normal rotation matrix
	float3x3 normal_rotation_matrix;
	normal_rotation_matrix[1] = input.normal.xyz;
	normal_rotation_matrix[2] = normalize(cross(float3(0.0, 0.0, -1.0), normal_rotation_matrix[1]));
	normal_rotation_matrix[0] = normalize(cross(normal_rotation_matrix[2], normal_rotation_matrix[1]));

	// need more high frequency bumps for plausible water surface, so creating normal defined by 2 instances of same bump texture
	float3 microbump_normal = normalize(2 * g_WaterBumpTexture.Sample(SamplerAnisotropicWrap, input.texcoord - g_WaterBumpTexcoordShift*0.2).gbr - float3 (1, -8, 1));
	microbump_normal += normalize(2 * g_WaterBumpTexture.Sample(SamplerAnisotropicWrap, input.texcoord*0.5 + g_WaterBumpTexcoordShift*0.05).gbr - float3 (1, -8, 1));

	// applying base normal rotation matrix to high frequency bump normal
	microbump_normal = mul(normalize(microbump_normal), normal_rotation_matrix);

	// simulating scattering/double refraction: light hits the side of wave, travels some distance in water, and leaves wave on the other side
	// it's difficult to do it physically correct without photon mapping/ray tracing, so using simple but plausible emulation below

	// only the crests of water waves generate double refracted light
	float scatter_factor = 2.5*max(0, input.positionWS.y*0.25 + 0.25);

	float3 pixel_to_light_vector = normalize(g_LightPosition - input.positionWS);
	float3 pixel_to_eye_vector = normalize(g_CameraPosition - input.positionWS);

	// the waves that lie between camera and light projection on water plane generate maximal amount of double refracted light 
	scatter_factor *= shadow_factor*pow(max(0.0, dot(normalize(float3(pixel_to_light_vector.x, 0.0, pixel_to_light_vector.z)), -pixel_to_eye_vector)), 2.0);

	// the slopes of waves that are oriented back to light generate maximal amount of double refracted light 
	scatter_factor *= pow(max(0.0, 1.0 - dot(pixel_to_light_vector, microbump_normal)), 8.0);

	// water crests gather more light than lobes, so more light is scattered under the crests
	scatter_factor += shadow_factor*1.5*g_WaterColorIntensity.y*max(0, input.positionWS.y + 1)*
	// the scattered light is best seen if observing direction is normal to slope surface
	max(0, dot(pixel_to_eye_vector, microbump_normal))*
	// fading scattered light out at distance and if viewing direction is vertical to avoid unnatural look
	max(0, 1 - pixel_to_eye_vector.y)*(300.0 / (300 + length(g_CameraPosition - input.positionWS)));

	// fading scatter out by 90% near shores so it looks better
	scatter_factor *= 0.1 + 0.9*input.depthmap_scaler.g;

	// calculating fresnel factor 
	float r = (1.2 - 1.0) / (1.2 + 1.0);
	float fresnel_factor = max(0.0, min(1.0, r + (1.0 - r)*pow(1.0 - dot(microbump_normal, pixel_to_eye_vector), 4)));

	// calculating specular factor
	float3 reflected_eye_to_pixel_vector = -pixel_to_eye_vector + 2 * dot(pixel_to_eye_vector, microbump_normal)*microbump_normal;

	// calculating diffuse intensity of water surface itself
	float diffuse_factor = g_WaterColorIntensity.x + g_WaterColorIntensity.y*max(0, dot(pixel_to_light_vector, microbump_normal));

	// calculating disturbance which has to be applied to planar reflections/refractions to give plausible results
	float4 disturbance_eyespace = mul(float4(microbump_normal.x, 0, microbump_normal.z, 0), g_ModelViewMatrix);

	float2 reflection_disturbance = float2(disturbance_eyespace.x, disturbance_eyespace.z)*0.03;

	float2 refraction_disturbance = float2(-disturbance_eyespace.x, disturbance_eyespace.y)*0.05*
	// fading out reflection disturbance at distance so reflection doesn't look noisy at distance
	(20.0 / (20 + length(g_CameraPosition - input.positionWS)));

	// calculating correction that shifts reflection up/down according to water wave Y position
	float4 projected_waveheight = mul(float4(input.positionWS.x, input.positionWS.y, input.positionWS.z, 1), g_ModelViewProjectionMatrix);
	float waveheight_correction = -0.5*projected_waveheight.y / projected_waveheight.w;
	projected_waveheight = mul(float4(input.positionWS.x, -0.8, input.positionWS.z, 1), g_ModelViewProjectionMatrix);
	waveheight_correction += 0.5*projected_waveheight.y / projected_waveheight.w;
	reflection_disturbance.y = max(-0.15, waveheight_correction + reflection_disturbance.y);

	float2 g_ScreenSizeInv = float2(0.000710, 0.001319);

	float g_ZNear = 1.0f;
	float g_ZFar = 25000.0f;

	// picking refraction depth at non-displaced point, need it to scale the refraction texture displacement amount according to water depth
	float refraction_depth = GetRefractionDepth(input.position.xy*g_ScreenSizeInv);
	refraction_depth = g_ZFar*g_ZNear / (g_ZFar - refraction_depth*(g_ZFar - g_ZNear));
	float4 vertex_in_viewspace = mul(float4(input.positionWS, 1), g_ModelViewMatrix);

	float water_depth = refraction_depth - vertex_in_viewspace.z;

	// scaling refraction texture displacement amount according to water depth, with some limit
	refraction_disturbance *= min(2, water_depth);

	// picking refraction depth again, now at displaced point, need it to calculate correct water depth
	refraction_depth = GetRefractionDepth(input.position.xy*g_ScreenSizeInv + refraction_disturbance);
	refraction_depth = g_ZFar*g_ZNear / (g_ZFar - refraction_depth*(g_ZFar - g_ZNear));
	vertex_in_viewspace = mul(float4(input.positionWS, 1), g_ModelViewMatrix);
	water_depth = refraction_depth - vertex_in_viewspace.z;

	// zeroing displacement for points where displaced position points at geometry which is actually closer to the camera than the water surface
	float conservative_refraction_depth = GetConservativeRefractionDepth(input.position.xy*g_ScreenSizeInv + refraction_disturbance);
	conservative_refraction_depth = g_ZFar*g_ZNear / (g_ZFar - conservative_refraction_depth*(g_ZFar - g_ZNear));
	vertex_in_viewspace = mul(float4(input.positionWS, 1), g_ModelViewMatrix);
	float conservative_water_depth = conservative_refraction_depth - vertex_in_viewspace.z;

	float nondisplaced_water_depth = water_depth;
	if (conservative_water_depth < 0)
	{
		refraction_disturbance = 0;
		water_depth = nondisplaced_water_depth;
	}
	water_depth = max(0, water_depth);
	
	float4 refraction_color = g_RefractionTexture.SampleLevel(SamplerLinearClamp, input.position.xy*g_ScreenSizeInv + refraction_disturbance, 0);

	// calculating water surface color and applying atmospheric fog to it
	float4 water_color = diffuse_factor*float4(g_WaterDeepColor, 1);
	water_color.rgb = lerp(CalculateFogColor(pixel_to_light_vector, pixel_to_eye_vector).rgb, water_color.rgb, min(1, exp(-length(g_CameraPosition - input.positionWS)*g_FogDensity)));

	// fading fresnel factor to 0 to soften water surface edges
	fresnel_factor *= min(1, water_depth*5.0);

	// fading refraction color to water color according to distance that refracted ray travels in water 
	refraction_color = lerp(water_color, refraction_color, min(1, 1.0*exp(-water_depth / 8.0)));

	float specular_factor = shadow_factor*fresnel_factor*pow(max(0, dot(pixel_to_light_vector, reflected_eye_to_pixel_vector)), g_WaterSpecularPower);

	// getting reflection and refraction color at disturbed texture coordinates
	float4 reflection_color = g_ReflectionTexture.SampleLevel(SamplerLinearClamp, float2(input.position.x*g_ScreenSizeInv.x, 1.0 - input.position.y*g_ScreenSizeInv.y) + reflection_disturbance, 0);

	// combining final water color
	float4 color;
	color.rgb = lerp(refraction_color.rgb, reflection_color.rgb, fresnel_factor);
	color.rgb += g_WaterSpecularIntensity*specular_factor*g_WaterSpecularColor*fresnel_factor;
	color.rgb += g_WaterScatterColor*scatter_factor;
	color.a = 1;
	return color;
}

float4 MainToBackBufferPS(PSIn_Quad input) : SV_Target
{
	float4 color;
	color.rgb = g_MainTexture.SampleLevel(SamplerLinearWrap, float2((input.texcoord.x - 0.5) / g_MainBufferSizeMultiplier + 0.5f, (input.texcoord.y - 0.5) / g_MainBufferSizeMultiplier + 0.5f), 0).rgb;
	color.a = 0;
	return color;
}


float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}