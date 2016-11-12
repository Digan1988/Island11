struct PSIn_Quad
{
	float4 position     : SV_Position;
	float2 texcoord     : TEXCOORD0;
};

Texture2D g_WaterBumpTexture : register(t0);
SamplerState SamplerLinearWrap:register(s0);

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
	float2  g_WaterBumpTexcoordShift = float2(0.000914, 0.000457);

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
	float4 color = float4(1, 0, 0, 1);

	float3 g_CameraPosition = float3(365.000031, 3.000000, 166.000031);

	color.rgb = (CombineSimplifiedWaterNormal(g_CameraPosition + float3(input.texcoord.x*400.0f - 200.0f, 0, input.texcoord.y*400.0f - 200.0f), 0).rgb + float3(1.0f, 1.0f, 1.0f))*0.5f;
	
	//color.a = 0;
	return color;

	//return g_WaterBumpTexture.Sample(SamplerLinearWrap, input.texcoord);
}

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}