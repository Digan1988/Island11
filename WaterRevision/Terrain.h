#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "CommonStates.h"
#include "Camera.h"
#include "CBuffer.h"

using namespace DirectX;

#define main_buffer_size_multiplier			1.1f
#define reflection_buffer_size_multiplier   1.1f
#define refraction_buffer_size_multiplier   1.1f
#define shadowmap_resource_buffer_size_xy				4096
#define water_normalmap_resource_buffer_size_xy			2048
#define terrain_numpatches_1d				64

#define terrain_gridpoints					512
#define terrain_fractalinitialvalue			100.0f
#define terrain_fractalfactor				0.68f;
#define terrain_maxheight					30.0f 
#define terrain_minheight					-30.0f 
#define terrain_geometry_scale				1.0f
#define terrain_smoothsteps					40
#define terrain_rockfactor					0.95f
#define terrain_smoothfactor1				0.99f
#define terrain_smoothfactor2				0.10f
#define terrain_layerdef_map_texture_size				1024
#define terrain_height_underwater_start		-100.0f
#define terrain_height_underwater_end		-8.0f
#define terrain_height_sand_start			-30.0f
#define terrain_height_sand_end				1.7f
#define terrain_height_grass_start			1.7f
#define terrain_height_grass_end			30.0f
#define terrain_slope_grass_start			0.96f
#define terrain_slope_rocks_start			0.85f
#define terrain_height_sand_start			-30.0f
#define terrain_height_sand_end				1.7f
#define terrain_height_rocks_start			-2.0f
#define terrain_depth_shadow_map_texture_size			512
#define sky_gridpoints						10
#define sky_texture_angle					0.425f

#define terrain_far_range terrain_gridpoints*terrain_geometry_scale

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) \
   if(x != NULL)        \
      {                    \
      x->Release();     \
      x = NULL;         \
      }
#endif

class Terrain
{
public:
	Terrain();
	~Terrain();
	void LoadTextures();
	void Initialize(ID3D11Device*);
	void createShaders();
	void ReCreateBuffers();
	void Render(Camera *, ID3D11RenderTargetView *colorBuffer, ID3D11DepthStencilView  *backBuffer, double time);
	void CreateTerrain();

	void renderTarrainToDepthBuffer(ID3D11DeviceContext* pContext, Camera *cam);
	void renderCaustics(ID3D11DeviceContext* pContext, Camera *cam);
	void renderReflection(ID3D11DeviceContext* pContext, Camera *cam);
	void renderTarrain(ID3D11DeviceContext* pContext, D3D11_VIEWPORT &main_Viewport, Camera *cam);
	void renderRefraction(ID3D11DeviceContext* pContext, D3D11_VIEWPORT &main_Viewport);
	void renderWater(ID3D11DeviceContext* pContext, D3D11_VIEWPORT &main_Viewport, Camera *cam);
	void renderToBackBuffer(ID3D11DeviceContext* pContext, ID3D11RenderTargetView *colorBuffer, ID3D11DepthStencilView  *backBuffer, D3D11_VIEWPORT &currentViewport);
	void renderSky(ID3D11DeviceContext* pContext);

	void SetupLightView(Camera *);
	void SetupLightView2(Camera *);
	void SetupNormalView(Camera *);
	void SetupReflectionView(Camera *);

	float BackbufferWidth;
	float BackbufferHeight;
	UINT MultiSampleCount;
	UINT MultiSampleQuality;
private:
	ID3D11Device* pDevice;
	std::unique_ptr<DirectX::CommonStates> m_states;

	//���������� �����
	ID3D11Buffer* cbPerObjectBuffer;

	//Rasterizer States
	//renderTarrain, renderWater, renderReflection, renderTarrainToDepthBuffer
	ID3D11RasterizerState *pCullBackMS = nullptr;
	//renderRefraction,renderCaustics
	ID3D11RasterizerState *pNoCullMS = nullptr;

	//DepthStencil States
	//renderTarrain, renderWater, renderReflection, renderTarrainToDepthBuffer
	ID3D11DepthStencilState *pDepthNormal = nullptr;
	//renderRefraction, renderCaustics
	ID3D11DepthStencilState *pNoDepthStencil = nullptr;

	//Blend State
	//renderTarrain, renderRefraction, renderWater, renderToBackBuffer, renderReflection, renderCaustics, renderTarrainToDepthBuffer
	ID3D11BlendState *pNoBlending = nullptr;

	//Input layouts
	//PATCH_PARAMETERS: renderTarrain, renderWater, renderReflection, renderTarrainToDepthBuffer
	ID3D11InputLayout   *heightfield_inputlayout;
	//POSITION | TEXCOORD: renderRefraction, renderSky, renderCaustics
	ID3D11InputLayout   *trianglestrip_inputlayout;

	//shaders
	//renderTarrain, renderWater, renderReflection, renderTarrainToDepthBuffer
	ID3D11VertexShader* PassThroughVS;
	//renderCaustics
	ID3D11VertexShader* WaterNormalmapCombineVS;
	//renderSky
	ID3D11VertexShader* SkyVS;
	//renderRefraction, renderToBackBuffer
	ID3D11VertexShader* FullScreenQuadVS;

	//renderTarrain, renderWater, renderReflection, renderTarrainToDepthBuffer
	ID3D11HullShader* PatchHS;

	//renderTarrain, renderReflection
	ID3D11DomainShader* HeightFieldPatchDS;
	//renderTarrainToDepthBuffer
	ID3D11DomainShader* WaterPatchDS;

	//renderTarrain, renderReflection
	ID3D11PixelShader* HeightFieldPatchPS;
	//renderTarrainToDepthBuffer
	ID3D11PixelShader* ColorPS;
	//renderCaustics
	ID3D11PixelShader* WaterNormalmapCombinePS;
	//renderSky
	ID3D11PixelShader* SkyPS;
	//renderRefraction
	ID3D11PixelShader* RefractionDepthManualResolveMS1PS;
	////
	//ID3D11PixelShader* RefractionDepthManualResolveMS2PS;
	////
	//ID3D11PixelShader* RefractionDepthManualResolveMS4PS;
	//renderWater
	ID3D11PixelShader* WaterPatchPS;
	//renderToBackBuffer
	ID3D11PixelShader* MainToBackBufferPS;

	//��������: src ��� ResolveSubresource � renderToBackBuffer � renderRefraction (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
	ID3D11Texture2D	*main_color_resource;
	//������������ ������ ��� render target, ������������� main_color_resource
	ID3D11ShaderResourceView *main_color_resourceSRV;
	//renderWater: � OMSetRenderTargets c main_depth_resourceDSV (rt �� ���������), renderTarrain: � OMSetRenderTargets c main_depth_resourceDSV (rt ���������), main_color_resource - �������� ����� RT
	ID3D11RenderTargetView   *main_color_resourceRTV;
	//renderToBackBuffer: dst ��� ResolveSubresource (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
	ID3D11Texture2D			 *main_color_resource_resolved;
	//������������� main_color_resource_resolved, renderToBackBuffer: �������� � ������ MainToBackBufferPS, ���� 12
	ID3D11ShaderResourceView *g_MainTexture;
	//����������� ��� main_depth_resourceDSV (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE)
	ID3D11Texture2D			 *main_depth_resource;
	//renderTarrain: DSV � OMSetRenderTargets (���������), renderWater: DSV � OMSetRenderTargets (�� ���������)
	ID3D11DepthStencilView   *main_depth_resourceDSV;
	//������������� main_depth_resource, renderRefraction: �������� � ������ RefractionDepthManualResolveMS1PS, ���� 13
	ID3D11ShaderResourceView *main_depth_resourceSRV;
	//����������� ��� reflection_color_resourceRTV (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
	ID3D11Texture2D			 *reflection_color_resource;
	//������������� reflection_color_resource, renderWater: �������� � ������ WaterPatchPS, ���� 10
	ID3D11ShaderResourceView *g_ReflectionTexture;
	//reflection_color_resource - �������� ����� RT, renderReflection: � OMSetRenderTargets ������ � reflection_depth_resourceDSV, RT ���������
	ID3D11RenderTargetView   *reflection_color_resourceRTV;
	//����������� ��� refraction_color_resourceRTV, renderRefraction: dst ��� ResolveSubresource (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
	ID3D11Texture2D			 *refraction_color_resource;
	//������������� refraction_color_resource, renderWater: �������� � ������ WaterPatchPS, ���� 11
	ID3D11ShaderResourceView *g_RefractionTexture;
	//refraction_color_resource - �������� ����� RT, �� ������������ ?
	//ID3D11RenderTargetView   *refraction_color_resourceRTV;
	//����������� ��� reflection_depth_resourceDSV (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE)
	ID3D11Texture2D			 *reflection_depth_resource;
	//renderReflection: � OMSetRenderTargets ������ � reflection_color_resourceRTV, DSV ���������
	ID3D11DepthStencilView   *reflection_depth_resourceDSV;
	//����������� ��� refraction_depth_resourceRTV (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
	ID3D11Texture2D			 *refraction_depth_resource;
	//refraction_depth_resource - �������� ����� RT, renderRefraction: � OMSetRenderTargets, DSV NULL
	ID3D11RenderTargetView   *refraction_depth_resourceRTV;
	//������������� refraction_depth_resource, renderWater: �������� � ������ WaterPatchPS, ���� 9 
	ID3D11ShaderResourceView *g_RefractionDepthTextureResolved;
	//����������� ��� shadowmap_resourceDSV (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE)
	ID3D11Texture2D			 *shadowmap_resource;
	//������������� shadowmap_resource, renderTarrain: HeightFieldPatchPS ���� 6, renderWater: WaterPatchPS ���� 6, renderReflection: HeightFieldPatchPS ���� 6
	ID3D11ShaderResourceView *g_DepthTexture;
	//renderTarrainToDepthBuffer: � OMSetRenderTargets, DSV ���������, RTV NULL
	ID3D11DepthStencilView   *shadowmap_resourceDSV;
	//����������� ��� water_normalmap_resourceRTV
	ID3D11Texture2D			 *water_normalmap_resource;
	//������������� water_normalmap_resource, renderTarrain: HeightFieldPatchDS ���� 4, renderWater: WaterPatchDS ���� 4, 
	ID3D11ShaderResourceView *g_WaterNormalMapTexture;
	//water_normalmap_resource - �������� ����� RT, renderCaustics: � OMSetRenderTargets, RTV ���������, DSV NULL
	ID3D11RenderTargetView   *water_normalmap_resourceRTV;

	CBuffer cbuffer;

	float	 height[terrain_gridpoints + 1][terrain_gridpoints + 1];
	XMFLOAT3 normal[terrain_gridpoints + 1][terrain_gridpoints + 1];

	//�������������� �������� ���������
	ID3D11Texture2D		*layerdef_texture;
	//������������� layerdef_texture, renderTarrain, renderRefraction, renderReflection, renderTarrainToDepthBuffer: � HeightFieldPatchDS ���� 1
	ID3D11ShaderResourceView *g_LayerdefTexture;
	//�������������� ����� �����
	ID3D11Texture2D		*heightmap_texture;
	//������������� heightmap_texture, renderTarrain, renderRefraction, renderReflection,renderTarrainToDepthBuffer: � HeightFieldPatchDS ���� 0, PatchHS ���� 0
	ID3D11ShaderResourceView *g_HeightfieldTexture;
	//depthmap
	ID3D11Texture2D		*depthmap_texture;
	//������������� depthmap_texture, renderWater: WaterPatchDS ���� 5
	ID3D11ShaderResourceView *g_DepthMapTexture;

	//renderTarrain, renderRefraction, renderWater, renderReflection, renderCaustics, renderTarrainToDepthBuffer: ��������� ����� � �������
	ID3D11Buffer		*heightfield_vertexbuffer;
	//renderSky: ��������� ����� ����
	ID3D11Buffer		*sky_vertexbuffer;

	//renderTarrain, renderReflection, renderTarrainToDepthBuffer: HeightFieldPatchDS ���� 3
	ID3D11ShaderResourceView *g_RockBumpTexture;
	//renderTarrain, renderReflection, renderTarrainToDepthBuffer: HeightFieldPatchDS � ���� 2
	ID3D11ShaderResourceView *g_SandBumpTexture;
	//renderWater: WaterPatchDS ���� 6
	ID3D11ShaderResourceView *g_WaterBumpTexture;

	//renderTarrain, renderReflection: HeightFieldPatchPS ���� 0
	ID3D11ShaderResourceView *g_SandMicroBumpTexture;
	//renderTarrain, renderReflection: HeightFieldPatchPS ���� 1
	ID3D11ShaderResourceView *g_RockMicroBumpTexture;
	//renderTarrain, renderReflection: HeightFieldPatchPS ���� 2
	ID3D11ShaderResourceView *g_SlopeDiffuseTexture;
	//renderTarrain, renderReflection: HeightFieldPatchPS ���� 3
	ID3D11ShaderResourceView *g_SandDiffuseTexture;
	//renderTarrain, renderReflection: HeightFieldPatchPS ���� 4
	ID3D11ShaderResourceView *g_RockDiffuseTexture;
	//renderTarrain, renderReflection: HeightFieldPatchPS ���� 5
	ID3D11ShaderResourceView *g_GrassDiffuseTexture;
	//renderSky: SkyPS ���� 8
	ID3D11ShaderResourceView *g_SkyTexture;

	ID3D11ShaderResourceView *nullSRV = nullptr;
};
