#pragma once
#include "SimpleMath.h"

using namespace DirectX;
using namespace SimpleMath;

struct CBuffer
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
	XMFLOAT4X4  g_ModelViewMatrix;

	XMFLOAT4X4  g_ModelViewProjectionMatrix;

	XMFLOAT4X4	g_ModelViewProjectionMatrixInv;

	XMFLOAT4X4  g_LightModelViewProjectionMatrix;

	XMFLOAT4X4  g_LightModelViewProjectionMatrixInv;//не используется?

	XMFLOAT3    g_CameraPosition;
	float		g_SkipCausticsCalculation;

	XMFLOAT3    g_CameraDirection;
	int			g_MSSamples;

	XMFLOAT3    g_LightPosition;
	float	    g_MainBufferSizeMultiplier;

	XMFLOAT2    g_WaterBumpTexcoordShift;
	XMFLOAT2    g_ScreenSizeInv;

	float		g_ZNear;
	float		g_ZFar;
	// constants defining visual appearance
	XMFLOAT2	g_DiffuseTexcoordScale = { 130.0, 130.0 };

	XMFLOAT2	g_RockBumpTexcoordScale = { 10.0, 10.0 };
	XMFLOAT2	g_SandBumpTexcoordScale = { 3.5, 3.5 };

	float		g_RockBumpHeightScale = 3.0;
	float		g_SandBumpHeightScale = 0.5;
	float       g_TerrainSpecularIntensity = 0.5;
	float		g_WaterHeightBumpScale = 1.0f;

	XMFLOAT2	g_WaterMicroBumpTexcoordScale = { 225, 225 };
	XMFLOAT2	g_WaterBumpTexcoordScale = { 7, 7 };

	XMFLOAT3    g_WaterDeepColor = { 0.1, 0.4, 0.7 };
	float       g_WaterSpecularIntensity = 350.0;
	XMFLOAT3    g_WaterScatterColor = { 0.3, 0.7, 0.6 };
	float       g_WaterSpecularPower = 1000;

	XMFLOAT3    g_WaterSpecularColor = { 1, 1, 1 };
	float		g_FogDensity = 1.0f / 700.0f;

	XMFLOAT2    g_WaterColorIntensity = { 0.1, 0.2 };
	XMFLOAT2	g_HeightFieldOrigin = XMFLOAT2(0, 0);

	XMFLOAT3    g_AtmosphereBrightColor = { 1.0, 1.1, 1.4 };
	float		g_HeightFieldSize = 512;

	XMFLOAT3    g_AtmosphereDarkColor = { 0.6, 0.6, 0.7 };
	float       unuse = 0;
};
