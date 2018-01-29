#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "CommonStates.h"

using namespace DirectX;

#define terrain_gridpoints					512
#define terrain_numpatches_1d				64
#define terrain_geometry_scale				1.0f
#define terrain_maxheight					30.0f 
#define terrain_minheight					-30.0f 
#define terrain_fractalfactor				0.68f;
#define terrain_fractalinitialvalue			100.0f
#define terrain_smoothfactor1				0.99f
#define terrain_smoothfactor2				0.10f
#define terrain_rockfactor					0.95f
#define terrain_smoothsteps					40

#define terrain_height_underwater_start		-100.0f
#define terrain_height_underwater_end		-8.0f
#define terrain_height_sand_start			-30.0f
#define terrain_height_sand_end				1.7f
#define terrain_height_grass_start			1.7f
#define terrain_height_grass_end			30.0f
#define terrain_height_rocks_start			-2.0f
#define terrain_height_trees_start			4.0f
#define terrain_height_trees_end			30.0f
#define terrain_slope_grass_start			0.96f
#define terrain_slope_rocks_start			0.85f

#define terrain_far_range terrain_gridpoints*terrain_geometry_scale

#define shadowmap_resource_buffer_size_xy				4096
#define water_normalmap_resource_buffer_size_xy			2048
#define terrain_layerdef_map_texture_size				1024
#define terrain_depth_shadow_map_texture_size			512

#define sky_gridpoints						10
#define sky_texture_angle					0.425f

#define main_buffer_size_multiplier			1.1f
#define reflection_buffer_size_multiplier   1.1f
#define refraction_buffer_size_multiplier   1.1f

#define scene_z_near						1.0f
#define scene_z_far							25000.0f
#define camera_fov							110.0f

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) \
   if(x != NULL)        \
      {                    \
      x->Release();     \
      x = NULL;         \
      }
#endif

#ifndef SAFE_ARRAY_DELETE
#define SAFE_ARRAY_DELETE(x) \
   if(x != NULL)             \
      {                         \
      delete[] x;            \
      x = NULL;              \
      }
#endif


class Camera
{
public:
	Camera() {}
	~Camera(){}
	void init(XMFLOAT4X4 &view, XMFLOAT4X4 &proj, XMFLOAT3 &eyePt, XMFLOAT3 &lookAtPt)
	{
		_view = view;
		_proj = proj;
		_eyePt = XMFLOAT3(eyePt.x, eyePt.y, eyePt.z);
		_lookAtPt = XMFLOAT3(lookAtPt.x, lookAtPt.y, lookAtPt.z);

		XMStoreFloat4x4(&_world, XMMatrixIdentity());
	}
	void update(XMFLOAT4X4 &view, XMFLOAT3 &eyePt, XMFLOAT3 &lookAtPt)
	{
		_view = view;
		_eyePt = XMFLOAT3(eyePt.x, eyePt.y, eyePt.z);
		_lookAtPt = XMFLOAT3(lookAtPt.x, lookAtPt.y, lookAtPt.z);

		XMMATRIX mView = XMLoadFloat4x4(&_view);
		XMMATRIX worldMat = XMMatrixInverse(nullptr, mView);

		XMStoreFloat4x4(&_world, worldMat);
	}
	XMFLOAT3 GetEyePt() { return _eyePt; }
	XMFLOAT3 GetLookAtPt() { return _lookAtPt; }
	XMFLOAT4X4 GetViewMatrix() { return _view; }
	XMFLOAT4X4 GetWorldMatrix() { return _world; }
	XMFLOAT4X4 GetProjMatrix() { return _proj; }
private:
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _proj;
	XMFLOAT4X4 _world;
	XMFLOAT4X4 _reflectionViewMatrix;
	XMFLOAT3 _eyePt;
	XMFLOAT3 _lookAtPt;
};

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

	XMFLOAT4X4  g_LightModelViewProjectionMatrixInv;

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

class Terrain
{
public:
	Terrain();
	void Initialize(ID3D11Device*);
	void DeInitialize();
	void ReCreateBuffers();
	void LoadTextures();
	void createShaders();
	void Render(Camera *, ID3D11RenderTargetView *colorBuffer, ID3D11DepthStencilView  *backBuffer, double time);
	void CreateTerrain();

	void renderTarrain(ID3D11DeviceContext* pContext, D3D11_VIEWPORT &main_Viewport, Camera *cam);
	void renderRefraction(ID3D11DeviceContext* pContext, D3D11_VIEWPORT &main_Viewport);
	void renderWater(ID3D11DeviceContext* pContext, D3D11_VIEWPORT &main_Viewport, Camera *cam);
	void renderToBackBuffer(ID3D11DeviceContext* pContext, ID3D11RenderTargetView *colorBuffer, ID3D11DepthStencilView  *backBuffer, D3D11_VIEWPORT &currentViewport);
	void renderSky(ID3D11DeviceContext* pContext);
	void renderReflection(ID3D11DeviceContext* pContext, Camera *cam);
	void renderCaustics(ID3D11DeviceContext* pContext, Camera *cam);
	void renderTarrainToDepthBuffer(ID3D11DeviceContext* pContext, Camera *cam);

	float DynamicTesselationFactor;
	float StaticTesselationFactor;
	void SetupNormalView(Camera *);
	void SetupReflectionView(Camera *);
	void SetupRefractionView(Camera *);
	void SetupLightView(Camera *);
	void SetupLightView2(Camera *);
	float BackbufferWidth;
	float BackbufferHeight;

	UINT MultiSampleCount;
	UINT MultiSampleQuality;

	//ID3D11Texture2D		*rock_bump_texture;
	ID3D11ShaderResourceView *g_RockBumpTexture;

	//ID3D11Texture2D		*rock_microbump_texture;
	ID3D11ShaderResourceView *g_RockMicroBumpTexture;

	//ID3D11Texture2D		*rock_diffuse_texture;
	ID3D11ShaderResourceView *g_RockDiffuseTexture;

	//ID3D11Texture2D		*sand_bump_texture;
	ID3D11ShaderResourceView *g_SandBumpTexture;

	//ID3D11Texture2D		*sand_microbump_texture;
	ID3D11ShaderResourceView *g_SandMicroBumpTexture;

	//ID3D11Texture2D		*sand_diffuse_texture;
	ID3D11ShaderResourceView *g_SandDiffuseTexture;

	//ID3D11Texture2D		*grass_diffuse_texture;
	ID3D11ShaderResourceView *g_GrassDiffuseTexture;

	//ID3D11Texture2D		*slope_diffuse_texture;
	ID3D11ShaderResourceView *g_SlopeDiffuseTexture;

	//ID3D11Texture2D		*water_bump_texture;
	ID3D11ShaderResourceView *g_WaterBumpTexture;

	//ID3D11Texture2D		*sky_texture;
	ID3D11ShaderResourceView *g_SkyTexture;

	ID3D11Texture2D			 *reflection_color_resource;
	ID3D11ShaderResourceView *g_ReflectionTexture;
	ID3D11RenderTargetView   *reflection_color_resourceRTV;

	ID3D11Texture2D			 *refraction_color_resource;
	ID3D11ShaderResourceView *g_RefractionTexture;
	//ID3D11RenderTargetView   *refraction_color_resourceRTV;

	ID3D11Texture2D			 *shadowmap_resource;
	ID3D11ShaderResourceView *g_DepthTexture;
	ID3D11DepthStencilView   *shadowmap_resourceDSV;

	ID3D11Texture2D			 *reflection_depth_resource;
	ID3D11DepthStencilView   *reflection_depth_resourceDSV;


	ID3D11Texture2D			 *refraction_depth_resource;
	ID3D11RenderTargetView   *refraction_depth_resourceRTV;
	ID3D11ShaderResourceView *g_RefractionDepthTextureResolved;

	ID3D11Texture2D			 *water_normalmap_resource;
	ID3D11ShaderResourceView *g_WaterNormalMapTexture;
	ID3D11RenderTargetView   *water_normalmap_resourceRTV;

	ID3D11Texture2D			 *main_color_resource;
	ID3D11ShaderResourceView *main_color_resourceSRV;
	ID3D11RenderTargetView   *main_color_resourceRTV;

	ID3D11Texture2D			 *main_depth_resource;
	ID3D11DepthStencilView   *main_depth_resourceDSV;
	ID3D11ShaderResourceView *main_depth_resourceSRV;
	ID3D11Texture2D			 *main_color_resource_resolved;
	ID3D11ShaderResourceView *g_MainTexture;

	ID3D11ShaderResourceView *nullSRV;

	ID3D11Device* pDevice;

	float	 height[terrain_gridpoints + 1][terrain_gridpoints + 1];
	XMFLOAT3 normal[terrain_gridpoints + 1][terrain_gridpoints + 1];
	XMFLOAT3 tangent[terrain_gridpoints + 1][terrain_gridpoints + 1];
	XMFLOAT3 binormal[terrain_gridpoints + 1][terrain_gridpoints + 1];

	ID3D11Texture2D		*heightmap_texture;
	ID3D11ShaderResourceView *g_HeightfieldTexture;

	ID3D11Texture2D		*layerdef_texture;
	ID3D11ShaderResourceView *g_LayerdefTexture;

	ID3D11Texture2D		*depthmap_texture;
	ID3D11ShaderResourceView *g_DepthMapTexture;

	ID3D11Buffer		*heightfield_vertexbuffer;
	ID3D11Buffer		*sky_vertexbuffer;

	ID3D11InputLayout   *heightfield_inputlayout;
	ID3D11InputLayout   *trianglestrip_inputlayout;

	ID3D11ShaderResourceView *refraction_depth_resourceSRV;
private:

	CBuffer cbuffer;
	ID3D11Buffer* cbPerObjectBuffer;

	ID3D11VertexShader* PassThroughVS;
	ID3D11HullShader* PatchHS;
	ID3D11DomainShader* HeightFieldPatchDS;
	ID3D11PixelShader* HeightFieldPatchPS;

	ID3D11PixelShader* ColorPS;

	ID3D11VertexShader* WaterNormalmapCombineVS;
	ID3D11PixelShader* WaterNormalmapCombinePS;

	ID3D11VertexShader* SkyVS;
	ID3D11PixelShader* SkyPS;

	ID3D11VertexShader* FullScreenQuadVS;

	ID3D11PixelShader* RefractionDepthManualResolveMS1PS;

	ID3D11PixelShader* RefractionDepthManualResolveMS2PS;

	ID3D11PixelShader* RefractionDepthManualResolveMS4PS;

	ID3D11DomainShader* WaterPatchDS;
	ID3D11PixelShader* WaterPatchPS;

	ID3D11PixelShader* MainToBackBufferPS;

	std::unique_ptr<DirectX::CommonStates> m_states;


	ID3D11RasterizerState *pCullBackMS = nullptr;
	ID3D11DepthStencilState *pDepthNormal = nullptr;
	ID3D11BlendState *pNoBlending = nullptr;

	ID3D11RasterizerState *pNoCullMS = nullptr;
	ID3D11DepthStencilState *pNoDepthStencil = nullptr;
};

float bilinear_interpolation(float fx, float fy, float a, float b, float c, float d);

