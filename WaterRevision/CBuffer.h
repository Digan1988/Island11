#pragma once
#include "SimpleMath.h"

using namespace DirectX;
using namespace SimpleMath;

struct CBuffer
{
	// rendering control variables
	float		g_RenderCaustics;//HeightFieldPatchDS
	float		g_UseDynamicLOD;//PatchConstantHS,HeightFieldPatchDS, WaterPatchDS
	float		g_FrustumCullInHS;//PatchConstantHS
	float       g_DynamicTessFactor;//PatchConstantHS,HeightFieldPatchDS, WaterPatchDS

	float       g_StaticTessFactor;//PatchConstantHS,HeightFieldPatchDS, WaterPatchDS
	float		g_TerrainBeingRendered;//PatchConstantHS
	float		g_HalfSpaceCullSign;//HeightFieldPatchPS, 
	float		g_HalfSpaceCullPosition;//HeightFieldPatchPS

	// view/time dependent variables
	XMFLOAT4X4  g_ModelViewMatrix;//WaterPatchPS

	XMFLOAT4X4  g_ModelViewProjectionMatrix;//HeightFieldPatchDS,WaterPatchDS,PatchConstantHS,WaterPatchPS,SkyVS

	XMFLOAT4X4	g_ModelViewProjectionMatrixInv;//не используется?

	XMFLOAT4X4  g_LightModelViewProjectionMatrix;//HeightFieldPatchPS,WaterPatchPS

	XMFLOAT4X4  g_LightModelViewProjectionMatrixInv;//не используется?

	XMFLOAT3    g_CameraPosition;//HeightFieldPatchDS,WaterPatchDS,PatchConstantHS,HeightFieldPatchPS,WaterNormalmapCombinePS,SkyPS,WaterPatchPS
	float		g_SkipCausticsCalculation;//HeightFieldPatchDS

	XMFLOAT3    g_CameraDirection;//PatchConstantHS
	int			g_MSSamples;//не используется?

	XMFLOAT3    g_LightPosition;//HeightFieldPatchDS,HeightFieldPatchPS,SkyPS,WaterPatchPS
	float	    g_MainBufferSizeMultiplier;//MainToBackBufferPS

	XMFLOAT2    g_WaterBumpTexcoordShift;//WaterPatchDS,WaterNormalmapCombinePS,WaterPatchPS
	XMFLOAT2    g_ScreenSizeInv;//WaterPatchPS

	float		g_ZNear;//WaterPatchPS
	float		g_ZFar;//WaterPatchPS
	// constants defining visual appearance
	XMFLOAT2	g_DiffuseTexcoordScale = { 130.0, 130.0 };//HeightFieldPatchDS

	XMFLOAT2	g_RockBumpTexcoordScale = { 10.0, 10.0 };//HeightFieldPatchDS
	XMFLOAT2	g_SandBumpTexcoordScale = { 3.5, 3.5 };//HeightFieldPatchDS

	float		g_RockBumpHeightScale = 3.0;//HeightFieldPatchDS
	float		g_SandBumpHeightScale = 0.5;//HeightFieldPatchDS
	float       g_TerrainSpecularIntensity = 0.5;//не используется?
	float		g_WaterHeightBumpScale = 1.0f;//HeightFieldPatchDS,WaterPatchDS

	XMFLOAT2	g_WaterMicroBumpTexcoordScale = { 225, 225 };//WaterPatchDS
	XMFLOAT2	g_WaterBumpTexcoordScale = { 7, 7 };//WaterPatchDS,WaterNormalmapCombinePS

	XMFLOAT3    g_WaterDeepColor = { 0.1, 0.4, 0.7 };//WaterPatchPS
	float       g_WaterSpecularIntensity = 350.0;//WaterPatchPS
	XMFLOAT3    g_WaterScatterColor = { 0.3, 0.7, 0.6 };//WaterPatchPS
	float       g_WaterSpecularPower = 1000;//WaterPatchPS

	XMFLOAT3    g_WaterSpecularColor = { 1, 1, 1 };//WaterPatchPS
	float		g_FogDensity = 1.0f / 700.0f;//HeightFieldPatchPS,WaterPatchPS

	XMFLOAT2    g_WaterColorIntensity = { 0.1, 0.2 };//WaterPatchPS
	XMFLOAT2	g_HeightFieldOrigin = XMFLOAT2(0, 0);//не используется?

	XMFLOAT3    g_AtmosphereBrightColor = { 1.0, 1.1, 1.4 };//HeightFieldPatchPS, SkyPS,WaterPatchPS
	float		g_HeightFieldSize = 512;//HeightFieldPatchDS,WaterPatchDS,PatchHS(PatchConstantHS),WaterNormalmapCombinePS

	XMFLOAT3    g_AtmosphereDarkColor = { 0.6, 0.6, 0.7 };//HeightFieldPatchPS,SkyPS,WaterPatchPS
	float       unuse = 0;
};

struct CBufferHS
{
	//PatchConstantHS
	XMFLOAT4X4  g_ModelViewProjectionMatrix;
	XMFLOAT3    g_CameraPosition;
	float		g_UseDynamicLOD;
	XMFLOAT3    g_CameraDirection;
	float		g_FrustumCullInHS;
	float       g_DynamicTessFactor;
	float       g_StaticTessFactor;
	float		g_TerrainBeingRendered;
	float		g_HeightFieldSize = 512;
};

struct CBufferDS
{
	//HeightFieldPatchDS, WaterPatchDS
	XMFLOAT4X4  g_ModelViewProjectionMatrix;
	XMFLOAT3    g_CameraPosition;
	float		g_UseDynamicLOD;
	float       g_DynamicTessFactor;
	float       g_StaticTessFactor;
	float		g_WaterHeightBumpScale = 1.0f;
	float		g_HeightFieldSize = 512;
	//HeightFieldPatchDS
	XMFLOAT3    g_LightPosition;
	float		g_RenderCaustics;
	XMFLOAT2	g_DiffuseTexcoordScale = { 130.0, 130.0 };
	XMFLOAT2	g_RockBumpTexcoordScale = { 10.0, 10.0 };
	XMFLOAT2	g_SandBumpTexcoordScale = { 3.5, 3.5 };
	float		g_SkipCausticsCalculation;
	float		g_RockBumpHeightScale = 3.0;
	float		g_SandBumpHeightScale = 0.5;
	XMFLOAT3	UNUSED;
	//WaterPatchDS
	XMFLOAT2    g_WaterBumpTexcoordShift;
	XMFLOAT2	g_WaterMicroBumpTexcoordScale = { 225, 225 };
	XMFLOAT2	g_WaterBumpTexcoordScale = { 7, 7 };
	XMFLOAT2	UNUSED2;
};

struct CBufferPS
{
	//HeightFieldPatchPS,WaterPatchPS
	XMFLOAT4X4  g_LightModelViewProjectionMatrix;
	float		g_FogDensity = 1.0f / 700.0f;
	XMFLOAT3	UNUSED;
	//HeightFieldPatchPS, WaterPatchPS,SkyPS,WaterNormalmapCombinePS
	XMFLOAT3    g_CameraPosition;
	float UNUSED2;
	//HeightFieldPatchPS,WaterPatchPS,SkyPS
	XMFLOAT3    g_LightPosition;
	float UNUSED3;
	XMFLOAT3    g_AtmosphereBrightColor = { 1.0, 1.1, 1.4 };
	float UNUSED4;
	XMFLOAT3    g_AtmosphereDarkColor = { 0.6, 0.6, 0.7 };
	float UNUSED5;
	//WaterPatchPS,WaterNormalmapCombinePS
	XMFLOAT2    g_WaterBumpTexcoordShift;
	//HeightFieldPatchPS
	float		g_HalfSpaceCullSign;
	float		g_HalfSpaceCullPosition;
	//WaterPatchPS
	XMFLOAT4X4  g_ModelViewMatrix;
	XMFLOAT4X4  g_ModelViewProjectionMatrix;
	XMFLOAT2    g_ScreenSizeInv;
	XMFLOAT2    g_WaterColorIntensity = { 0.1, 0.2 };
	XMFLOAT3    g_WaterDeepColor = { 0.1, 0.4, 0.7 };
	float		g_ZNear;
	XMFLOAT3    g_WaterScatterColor = { 0.3, 0.7, 0.6 };
	float		g_ZFar;
	XMFLOAT3    g_WaterSpecularColor = { 1, 1, 1 };
	float       g_WaterSpecularIntensity = 350.0;
	float       g_WaterSpecularPower = 1000;
	//WaterNormalmapCombinePS
	XMFLOAT2	g_WaterBumpTexcoordScale = { 7, 7 };
	float		g_HeightFieldSize = 512;
};