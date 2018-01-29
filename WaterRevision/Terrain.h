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

	//констатный буфер
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

	//текстура: src для ResolveSubresource в renderToBackBuffer и renderRefraction (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
	ID3D11Texture2D	*main_color_resource;
	//используется только для render target, представление main_color_resource
	ID3D11ShaderResourceView *main_color_resourceSRV;
	//renderWater: в OMSetRenderTargets c main_depth_resourceDSV (rt не очищается), renderTarrain: в OMSetRenderTargets c main_depth_resourceDSV (rt очищается), main_color_resource - текстура этого RT
	ID3D11RenderTargetView   *main_color_resourceRTV;
	//renderToBackBuffer: dst для ResolveSubresource (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
	ID3D11Texture2D			 *main_color_resource_resolved;
	//представление main_color_resource_resolved, renderToBackBuffer: текстура в шейдер MainToBackBufferPS, слот 12
	ID3D11ShaderResourceView *g_MainTexture;
	//поверхность для main_depth_resourceDSV (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE)
	ID3D11Texture2D			 *main_depth_resource;
	//renderTarrain: DSV в OMSetRenderTargets (очищается), renderWater: DSV в OMSetRenderTargets (не очищается)
	ID3D11DepthStencilView   *main_depth_resourceDSV;
	//представление main_depth_resource, renderRefraction: текстура в шейдер RefractionDepthManualResolveMS1PS, слот 13
	ID3D11ShaderResourceView *main_depth_resourceSRV;
	//поверхность для reflection_color_resourceRTV (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
	ID3D11Texture2D			 *reflection_color_resource;
	//представление reflection_color_resource, renderWater: текстура в шейдер WaterPatchPS, слот 10
	ID3D11ShaderResourceView *g_ReflectionTexture;
	//reflection_color_resource - текстура этого RT, renderReflection: в OMSetRenderTargets вместе с reflection_depth_resourceDSV, RT очищается
	ID3D11RenderTargetView   *reflection_color_resourceRTV;
	//поверхность для refraction_color_resourceRTV, renderRefraction: dst для ResolveSubresource (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
	ID3D11Texture2D			 *refraction_color_resource;
	//представление refraction_color_resource, renderWater: текстура в шейдер WaterPatchPS, слот 11
	ID3D11ShaderResourceView *g_RefractionTexture;
	//refraction_color_resource - текстура этого RT, не используется ?
	//ID3D11RenderTargetView   *refraction_color_resourceRTV;
	//поверхность для reflection_depth_resourceDSV (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE)
	ID3D11Texture2D			 *reflection_depth_resource;
	//renderReflection: в OMSetRenderTargets вместе с reflection_color_resourceRTV, DSV очищается
	ID3D11DepthStencilView   *reflection_depth_resourceDSV;
	//поверхность для refraction_depth_resourceRTV (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET)
	ID3D11Texture2D			 *refraction_depth_resource;
	//refraction_depth_resource - текстура этого RT, renderRefraction: в OMSetRenderTargets, DSV NULL
	ID3D11RenderTargetView   *refraction_depth_resourceRTV;
	//представление refraction_depth_resource, renderWater: текстура в шейдер WaterPatchPS, слот 9 
	ID3D11ShaderResourceView *g_RefractionDepthTextureResolved;
	//поверхность для shadowmap_resourceDSV (D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE)
	ID3D11Texture2D			 *shadowmap_resource;
	//представление shadowmap_resource, renderTarrain: HeightFieldPatchPS слот 6, renderWater: WaterPatchPS слот 6, renderReflection: HeightFieldPatchPS слот 6
	ID3D11ShaderResourceView *g_DepthTexture;
	//renderTarrainToDepthBuffer: в OMSetRenderTargets, DSV очищается, RTV NULL
	ID3D11DepthStencilView   *shadowmap_resourceDSV;
	//поверхность для water_normalmap_resourceRTV
	ID3D11Texture2D			 *water_normalmap_resource;
	//представление water_normalmap_resource, renderTarrain: HeightFieldPatchDS слот 4, renderWater: WaterPatchDS слот 4, 
	ID3D11ShaderResourceView *g_WaterNormalMapTexture;
	//water_normalmap_resource - текстура этого RT, renderCaustics: в OMSetRenderTargets, RTV очищается, DSV NULL
	ID3D11RenderTargetView   *water_normalmap_resourceRTV;

	CBuffer cbuffer;

	float	 height[terrain_gridpoints + 1][terrain_gridpoints + 1];
	XMFLOAT3 normal[terrain_gridpoints + 1][terrain_gridpoints + 1];

	//сгенерированая текстура ландшафта
	ID3D11Texture2D		*layerdef_texture;
	//представление layerdef_texture, renderTarrain, renderRefraction, renderReflection, renderTarrainToDepthBuffer: в HeightFieldPatchDS слот 1
	ID3D11ShaderResourceView *g_LayerdefTexture;
	//сгенерированая карта высот
	ID3D11Texture2D		*heightmap_texture;
	//представление heightmap_texture, renderTarrain, renderRefraction, renderReflection,renderTarrainToDepthBuffer: в HeightFieldPatchDS слот 0, PatchHS слот 0
	ID3D11ShaderResourceView *g_HeightfieldTexture;
	//depthmap
	ID3D11Texture2D		*depthmap_texture;
	//представление depthmap_texture, renderWater: WaterPatchDS слот 5
	ID3D11ShaderResourceView *g_DepthMapTexture;

	//renderTarrain, renderRefraction, renderWater, renderReflection, renderCaustics, renderTarrainToDepthBuffer: вершинный буфер с патчами
	ID3D11Buffer		*heightfield_vertexbuffer;
	//renderSky: вершинный буфер неба
	ID3D11Buffer		*sky_vertexbuffer;

	//renderTarrain, renderReflection, renderTarrainToDepthBuffer: HeightFieldPatchDS слот 3
	ID3D11ShaderResourceView *g_RockBumpTexture;
	//renderTarrain, renderReflection, renderTarrainToDepthBuffer: HeightFieldPatchDS в слот 2
	ID3D11ShaderResourceView *g_SandBumpTexture;
	//renderWater: WaterPatchDS слот 6
	ID3D11ShaderResourceView *g_WaterBumpTexture;

	//renderTarrain, renderReflection: HeightFieldPatchPS слот 0
	ID3D11ShaderResourceView *g_SandMicroBumpTexture;
	//renderTarrain, renderReflection: HeightFieldPatchPS слот 1
	ID3D11ShaderResourceView *g_RockMicroBumpTexture;
	//renderTarrain, renderReflection: HeightFieldPatchPS слот 2
	ID3D11ShaderResourceView *g_SlopeDiffuseTexture;
	//renderTarrain, renderReflection: HeightFieldPatchPS слот 3
	ID3D11ShaderResourceView *g_SandDiffuseTexture;
	//renderTarrain, renderReflection: HeightFieldPatchPS слот 4
	ID3D11ShaderResourceView *g_RockDiffuseTexture;
	//renderTarrain, renderReflection: HeightFieldPatchPS слот 5
	ID3D11ShaderResourceView *g_GrassDiffuseTexture;
	//renderSky: SkyPS слот 8
	ID3D11ShaderResourceView *g_SkyTexture;

	ID3D11ShaderResourceView *nullSRV = nullptr;
};
