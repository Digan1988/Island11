#include "Terrain.h"
#include <d3dcompiler.h>
#include "DDSTextureLoader.h"

using namespace DirectX;
using namespace std;

#pragma comment(lib, "D3DCompiler.lib")

#ifndef PI
#define PI 3.14159265358979323846f
#endif

Terrain::Terrain()
{
}

Terrain::~Terrain()
{
}

void Terrain::Initialize(ID3D11Device* device)
{
	pDevice = device;

	m_states = std::make_unique<CommonStates>(pDevice);

	//////////////////////////////
	//Создание константного буфера
	//////////////////////////////
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(CBuffer);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	HRESULT result = device->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);
	///////////////////////////////////////////////////////////////////////

	//////////////////////////////
	//Создание RasterizerState
	//////////////////////////////
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
	desc.CullMode = D3D11_CULL_BACK;
	desc.FrontCounterClockwise = TRUE;
	desc.MultisampleEnable = TRUE;
	desc.FillMode = D3D11_FILL_SOLID;
	result = device->CreateRasterizerState(&desc, &pCullBackMS);

	desc.CullMode = D3D11_CULL_NONE;
	desc.MultisampleEnable = TRUE;
	result = device->CreateRasterizerState(&desc, &pNoCullMS);
	///////////////////////////////////////////////////////////////////////

	//////////////////////////////
	//Создание DepthStencilState
	//////////////////////////////
	D3D11_DEPTH_STENCIL_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthDesc.DepthEnable = TRUE;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDesc.StencilEnable = FALSE;
	depthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	result = device->CreateDepthStencilState(&depthDesc, &pDepthNormal);

	depthDesc.DepthEnable = FALSE;
	result = device->CreateDepthStencilState(&depthDesc, &pNoDepthStencil);
	///////////////////////////////////////////////////////////////////////

	//////////////////////////////
	//Создание BlendState
	//////////////////////////////
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	const D3D11_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
	{
		FALSE,
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
		D3D11_COLOR_WRITE_ENABLE_ALL,
	};

	blendDesc.RenderTarget[0] = defaultRenderTargetBlendDesc;
	result = device->CreateBlendState(&blendDesc, &pNoBlending);
	///////////////////////////////////////////////////////////////////////


	CreateTerrain();
}
void Terrain::createShaders()
{
	ID3D10Blob* pErrorMessage = nullptr;

	ID3D10Blob* VS_Buffer = nullptr;
	//ID3D10Blob* HS_Buffer = nullptr;
	//ID3D10Blob* DS_Buffer = nullptr;
	//ID3D10Blob* PS_Buffer = nullptr;

	//VertexShader

	const D3D11_INPUT_ELEMENT_DESC TerrainLayout =
	{
		"PATCH_PARAMETERS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
	};

	const char* pTarget = "vs_4_0";

	ID3D10Blob* RenderHeightfieldVS_Buffer = nullptr;

	HRESULT result = D3DCompileFromFile(L"TerrainVS.hlsl", nullptr, nullptr, "PassThroughVS", pTarget, D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &RenderHeightfieldVS_Buffer, &pErrorMessage);

	result = pDevice->CreateInputLayout(&TerrainLayout, 1, RenderHeightfieldVS_Buffer->GetBufferPointer(), RenderHeightfieldVS_Buffer->GetBufferSize(), &heightfield_inputlayout);

	const D3D11_INPUT_ELEMENT_DESC SkyLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D10Blob* RenderSkyVS_Buffer = nullptr;

	result = D3DCompileFromFile(L"TerrainVS.hlsl", nullptr, nullptr, "SkyVS", pTarget, D3D10_SHADER_ENABLE_STRICTNESS, 0, &RenderSkyVS_Buffer, &pErrorMessage);

	result = pDevice->CreateInputLayout(SkyLayout, 2, RenderSkyVS_Buffer->GetBufferPointer(), RenderSkyVS_Buffer->GetBufferSize(), &trianglestrip_inputlayout);

	result = D3DCompileFromFile(L"TerrainVS.hlsl", nullptr, nullptr, "PassThroughVS", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &PassThroughVS);

	result = D3DCompileFromFile(L"TerrainVS.hlsl", nullptr, nullptr, "WaterNormalmapCombineVS", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &WaterNormalmapCombineVS);

	result = D3DCompileFromFile(L"TerrainVS.hlsl", nullptr, nullptr, "SkyVS", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &SkyVS);

	result = D3DCompileFromFile(L"TerrainVS.hlsl", nullptr, nullptr, "FullScreenQuadVS", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &FullScreenQuadVS);

	//HullShader
	result = D3DCompileFromFile(L"TerrainHS.hlsl", nullptr, nullptr, "PatchHS", "hs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreateHullShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &PatchHS);

	//DomainShader
	result = D3DCompileFromFile(L"TerrainDS.hlsl", nullptr, nullptr, "HeightFieldPatchDS", "ds_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreateDomainShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &HeightFieldPatchDS);

	result = D3DCompileFromFile(L"TerrainDS.hlsl", nullptr, nullptr, "WaterPatchDS", "ds_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreateDomainShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &WaterPatchDS);


	//PixelShader
	result = D3DCompileFromFile(L"TerrainPS.hlsl", nullptr, nullptr, "HeightFieldPatchPS", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreatePixelShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &HeightFieldPatchPS);

	result = D3DCompileFromFile(L"TerrainPS.hlsl", nullptr, nullptr, "ColorPS", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreatePixelShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &ColorPS);

	result = D3DCompileFromFile(L"TerrainPS.hlsl", nullptr, nullptr, "WaterNormalmapCombinePS", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreatePixelShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &WaterNormalmapCombinePS);

	result = D3DCompileFromFile(L"TerrainPS.hlsl", nullptr, nullptr, "SkyPS", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreatePixelShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &SkyPS);

	result = D3DCompileFromFile(L"TerrainPS.hlsl", nullptr, nullptr, "RefractionDepthManualResolvePS1", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreatePixelShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &RefractionDepthManualResolveMS1PS);

	//result = D3DCompileFromFile(L"TerrainPS.hlsl", nullptr, nullptr, "RefractionDepthManualResolvePS2", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	//result = pDevice->CreatePixelShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &RefractionDepthManualResolveMS2PS);

	//result = D3DCompileFromFile(L"TerrainPS.hlsl", nullptr, nullptr, "RefractionDepthManualResolvePS4", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	//result = pDevice->CreatePixelShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &RefractionDepthManualResolveMS4PS);

	result = D3DCompileFromFile(L"TerrainPS.hlsl", nullptr, nullptr, "WaterPatchPS", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreatePixelShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &WaterPatchPS);

	result = D3DCompileFromFile(L"TerrainPS.hlsl", nullptr, nullptr, "MainToBackBufferPS", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &VS_Buffer, &pErrorMessage);

	result = pDevice->CreatePixelShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, &MainToBackBufferPS);

	if (pErrorMessage)
	{
		char* compileErrors = static_cast<char*>(pErrorMessage->GetBufferPointer());

		compileErrors = compileErrors;
	}
}
void Terrain::ReCreateBuffers()
{
	D3D11_TEXTURE2D_DESC tex_desc;
	ZeroMemory(&tex_desc, sizeof(tex_desc));

	tex_desc.Width = (UINT)(BackbufferWidth*main_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*main_buffer_size_multiplier);
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = MultiSampleCount;
	tex_desc.SampleDesc.Quality = MultiSampleQuality;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	pDevice->CreateTexture2D(&tex_desc, NULL, &main_color_resource);

	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc;
	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));

	textureSRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(main_color_resource, &textureSRV_desc, &main_color_resourceSRV);
	pDevice->CreateRenderTargetView(main_color_resource, NULL, &main_color_resourceRTV);

	ZeroMemory(&tex_desc, sizeof(tex_desc));
	tex_desc.Width = (UINT)(BackbufferWidth*main_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*main_buffer_size_multiplier);
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	pDevice->CreateTexture2D(&tex_desc, NULL, &main_color_resource_resolved);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(main_color_resource_resolved, &textureSRV_desc, &g_MainTexture);

	ZeroMemory(&tex_desc, sizeof(tex_desc));
	tex_desc.Width = (UINT)(BackbufferWidth*main_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*main_buffer_size_multiplier);
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count = MultiSampleCount;
	tex_desc.SampleDesc.Quality = MultiSampleQuality;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	pDevice->CreateTexture2D(&tex_desc, NULL, &main_depth_resource);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	textureSRV_desc.Texture2D.MipLevels = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(main_depth_resource, &textureSRV_desc, &main_depth_resourceSRV);

	D3D11_DEPTH_STENCIL_VIEW_DESC DSV_desc;
	DSV_desc.Format = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice = 0;

	pDevice->CreateDepthStencilView(main_depth_resource, &DSV_desc, &main_depth_resourceDSV);

	ZeroMemory(&tex_desc, sizeof(tex_desc));
	tex_desc.Width = (UINT)(BackbufferWidth*reflection_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*reflection_buffer_size_multiplier);
	tex_desc.MipLevels = 1;// (UINT)max(1, log(max((float)tex_desc.Width, (float)tex_desc.Height)) / (float)log(2.0f));
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	pDevice->CreateTexture2D(&tex_desc, NULL, &reflection_color_resource);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(reflection_color_resource, &textureSRV_desc, &g_ReflectionTexture);

	pDevice->CreateRenderTargetView(reflection_color_resource, NULL, &reflection_color_resourceRTV);

	ZeroMemory(&tex_desc, sizeof(tex_desc));
	tex_desc.Width = (UINT)(BackbufferWidth*refraction_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*refraction_buffer_size_multiplier);
	tex_desc.MipLevels = 1;//(UINT)max(1, log(max((float)tex_desc.Width, (float)tex_desc.Height)) / (float)log(2.0f));
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	pDevice->CreateTexture2D(&tex_desc, NULL, &refraction_color_resource);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(refraction_color_resource, &textureSRV_desc, &g_RefractionTexture);

	//pDevice->CreateRenderTargetView(refraction_color_resource, NULL, &refraction_color_resourceRTV);

	ZeroMemory(&tex_desc, sizeof(tex_desc));
	tex_desc.Width = (UINT)(BackbufferWidth*reflection_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*reflection_buffer_size_multiplier);
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	pDevice->CreateTexture2D(&tex_desc, NULL, &reflection_depth_resource);

	pDevice->CreateDepthStencilView(reflection_depth_resource, &DSV_desc, &reflection_depth_resourceDSV);

	tex_desc.Width = (UINT)(BackbufferWidth*refraction_buffer_size_multiplier);
	tex_desc.Height = (UINT)(BackbufferHeight*refraction_buffer_size_multiplier);
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32_FLOAT;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	pDevice->CreateTexture2D(&tex_desc, NULL, &refraction_depth_resource);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(refraction_depth_resource, &textureSRV_desc, &g_RefractionDepthTextureResolved);

	pDevice->CreateRenderTargetView(refraction_depth_resource, NULL, &refraction_depth_resourceRTV);

	tex_desc.Width = shadowmap_resource_buffer_size_xy;
	tex_desc.Height = shadowmap_resource_buffer_size_xy;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	pDevice->CreateTexture2D(&tex_desc, NULL, &shadowmap_resource);

	DSV_desc.Format = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice = 0;

	pDevice->CreateDepthStencilView(shadowmap_resource, &DSV_desc, &shadowmap_resourceDSV);

	pDevice->CreateShaderResourceView(shadowmap_resource, &textureSRV_desc, &g_DepthTexture);

	tex_desc.Width = water_normalmap_resource_buffer_size_xy;
	tex_desc.Height = water_normalmap_resource_buffer_size_xy;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	pDevice->CreateTexture2D(&tex_desc, NULL, &water_normalmap_resource);

	textureSRV_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(water_normalmap_resource, &textureSRV_desc, &g_WaterNormalMapTexture);

	pDevice->CreateRenderTargetView(water_normalmap_resource, NULL, &water_normalmap_resourceRTV);
}

int gp_wrap(int a)
{
	if (a<0) return (a + terrain_gridpoints);
	if (a >= terrain_gridpoints) return (a - terrain_gridpoints);
	return a;
}

float bilinear_interpolation(float fx, float fy, float a, float b, float c, float d)
{
	float s1, s2, s3, s4;
	s1 = fx*fy;
	s2 = (1 - fx)*fy;
	s3 = (1 - fx)*(1 - fy);
	s4 = fx*(1 - fy);
	return((a*s3 + b*s4 + c*s1 + d*s2));
}

void Terrain::CreateTerrain()
{
	int i, j, k, l;
	float x, z;
	int ix, iz;
	float * backterrain;
	XMVECTOR vec1, vec2, vec3;
	int currentstep = terrain_gridpoints;
	float mv, rm;
	float offset = 0, yscale = 0, maxheight = 0, minheight = 0;

	float *height_linear_array;
	float *patches_rawdata;
	HRESULT result;
	D3D11_SUBRESOURCE_DATA subresource_data;
	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc;

	backterrain = (float *)malloc((terrain_gridpoints + 1)*(terrain_gridpoints + 1) * sizeof(float));
	rm = terrain_fractalinitialvalue;
	backterrain[0] = 0;
	backterrain[0 + terrain_gridpoints*terrain_gridpoints] = 0;
	backterrain[terrain_gridpoints] = 0;
	backterrain[terrain_gridpoints + terrain_gridpoints*terrain_gridpoints] = 0;
	currentstep = terrain_gridpoints;
	srand(12);

	// generating fractal terrain using square-diamond method
	while (currentstep>1)
	{
		//square step;
		i = 0;
		j = 0;

		while (i<terrain_gridpoints)
		{
			j = 0;
			while (j<terrain_gridpoints)
			{

				mv = backterrain[i + terrain_gridpoints*j];
				mv += backterrain[(i + currentstep) + terrain_gridpoints*j];
				mv += backterrain[(i + currentstep) + terrain_gridpoints*(j + currentstep)];
				mv += backterrain[i + terrain_gridpoints*(j + currentstep)];
				mv /= 4.0;
				backterrain[i + currentstep / 2 + terrain_gridpoints*(j + currentstep / 2)] = (float)(mv + rm*((rand() % 1000) / 1000.0f - 0.5f));
				j += currentstep;
			}
			i += currentstep;
		}

		//diamond step;
		i = 0;
		j = 0;

		while (i<terrain_gridpoints)
		{
			j = 0;
			while (j<terrain_gridpoints)
			{

				mv = 0;
				mv = backterrain[i + terrain_gridpoints*j];
				mv += backterrain[(i + currentstep) + terrain_gridpoints*j];
				mv += backterrain[(i + currentstep / 2) + terrain_gridpoints*(j + currentstep / 2)];
				mv += backterrain[i + currentstep / 2 + terrain_gridpoints*gp_wrap(j - currentstep / 2)];
				mv /= 4;
				backterrain[i + currentstep / 2 + terrain_gridpoints*j] = (float)(mv + rm*((rand() % 1000) / 1000.0f - 0.5f));

				mv = 0;
				mv = backterrain[i + terrain_gridpoints*j];
				mv += backterrain[i + terrain_gridpoints*(j + currentstep)];
				mv += backterrain[(i + currentstep / 2) + terrain_gridpoints*(j + currentstep / 2)];
				mv += backterrain[gp_wrap(i - currentstep / 2) + terrain_gridpoints*(j + currentstep / 2)];
				mv /= 4;
				backterrain[i + terrain_gridpoints*(j + currentstep / 2)] = (float)(mv + rm*((rand() % 1000) / 1000.0f - 0.5f));

				mv = 0;
				mv = backterrain[i + currentstep + terrain_gridpoints*j];
				mv += backterrain[i + currentstep + terrain_gridpoints*(j + currentstep)];
				mv += backterrain[(i + currentstep / 2) + terrain_gridpoints*(j + currentstep / 2)];
				mv += backterrain[gp_wrap(i + currentstep / 2 + currentstep) + terrain_gridpoints*(j + currentstep / 2)];
				mv /= 4;
				backterrain[i + currentstep + terrain_gridpoints*(j + currentstep / 2)] = (float)(mv + rm*((rand() % 1000) / 1000.0f - 0.5f));

				mv = 0;
				mv = backterrain[i + currentstep + terrain_gridpoints*(j + currentstep)];
				mv += backterrain[i + terrain_gridpoints*(j + currentstep)];
				mv += backterrain[(i + currentstep / 2) + terrain_gridpoints*(j + currentstep / 2)];
				mv += backterrain[i + currentstep / 2 + terrain_gridpoints*gp_wrap(j + currentstep / 2 + currentstep)];
				mv /= 4;
				backterrain[i + currentstep / 2 + terrain_gridpoints*(j + currentstep)] = (float)(mv + rm*((rand() % 1000) / 1000.0f - 0.5f));
				j += currentstep;
			}
			i += currentstep;
		}
		//changing current step;
		currentstep /= 2;
		rm *= terrain_fractalfactor;
	}

	// scaling to minheight..maxheight range
	for (i = 0; i<terrain_gridpoints + 1; i++)
		for (j = 0; j<terrain_gridpoints + 1; j++)
		{
			height[i][j] = backterrain[i + terrain_gridpoints*j];
		}
	maxheight = height[0][0];
	minheight = height[0][0];
	for (i = 0; i<terrain_gridpoints + 1; i++)
		for (j = 0; j<terrain_gridpoints + 1; j++)
		{
			if (height[i][j]>maxheight) maxheight = height[i][j];
			if (height[i][j]<minheight) minheight = height[i][j];
		}
	offset = minheight - terrain_minheight;
	yscale = (terrain_maxheight - terrain_minheight) / (maxheight - minheight);

	for (i = 0; i<terrain_gridpoints + 1; i++)
		for (j = 0; j<terrain_gridpoints + 1; j++)
		{
			height[i][j] -= minheight;
			height[i][j] *= yscale;
			height[i][j] += terrain_minheight;
		}

	// moving down edges of heightmap	
	for (i = 0; i<terrain_gridpoints + 1; i++)
		for (j = 0; j<terrain_gridpoints + 1; j++)
		{
			mv = (float)((i - terrain_gridpoints / 2.0f)*(i - terrain_gridpoints / 2.0f) + (j - terrain_gridpoints / 2.0f)*(j - terrain_gridpoints / 2.0f));
			rm = (float)((terrain_gridpoints*0.8f)*(terrain_gridpoints*0.8f) / 4.0f);
			if (mv>rm)
			{
				height[i][j] -= ((mv - rm) / 1000.0f)*terrain_geometry_scale;
			}
			if (height[i][j]<terrain_minheight)
			{
				height[i][j] = terrain_minheight;
			}
		}


	// terrain banks
	for (k = 0; k<10; k++)
	{
		for (i = 0; i<terrain_gridpoints + 1; i++)
			for (j = 0; j<terrain_gridpoints + 1; j++)
			{
				mv = height[i][j];
				if ((mv)>0.02f)
				{
					mv -= 0.02f;
				}
				if (mv<-0.02f)
				{
					mv += 0.02f;
				}
				height[i][j] = mv;
			}
	}

	// smoothing 
	for (k = 0; k<terrain_smoothsteps; k++)
	{
		for (i = 0; i<terrain_gridpoints + 1; i++)
			for (j = 0; j<terrain_gridpoints + 1; j++)
			{

				vec1.m128_f32[0] = 2 * terrain_geometry_scale;
				vec1.m128_f32[1] = terrain_geometry_scale*(height[gp_wrap(i + 1)][j] - height[gp_wrap(i - 1)][j]);
				vec1.m128_f32[2] = 0;

				vec2.m128_f32[0] = 0;
				vec2.m128_f32[1] = -terrain_geometry_scale*(height[i][gp_wrap(j + 1)] - height[i][gp_wrap(j - 1)]);
				vec2.m128_f32[2] = -2 * terrain_geometry_scale;

				vec3 = XMVector3Cross(vec1, vec2);
				vec3 = XMVector3Normalize(vec3);

				if (((vec3.m128_f32[1]>terrain_rockfactor) || (height[i][j]<1.2f)))
				{
					rm = terrain_smoothfactor1;
					mv = height[i][j] * (1.0f - rm) + rm*0.25f*(height[gp_wrap(i - 1)][j] + height[i][gp_wrap(j - 1)] + height[gp_wrap(i + 1)][j] + height[i][gp_wrap(j + 1)]);
					backterrain[i + terrain_gridpoints*j] = mv;
				}
				else
				{
					rm = terrain_smoothfactor2;
					mv = height[i][j] * (1.0f - rm) + rm*0.25f*(height[gp_wrap(i - 1)][j] + height[i][gp_wrap(j - 1)] + height[gp_wrap(i + 1)][j] + height[i][gp_wrap(j + 1)]);
					backterrain[i + terrain_gridpoints*j] = mv;
				}

			}
		for (i = 0; i<terrain_gridpoints + 1; i++)
			for (j = 0; j<terrain_gridpoints + 1; j++)
			{
				height[i][j] = (backterrain[i + terrain_gridpoints*j]);
			}
	}
	for (i = 0; i<terrain_gridpoints + 1; i++)
		for (j = 0; j<terrain_gridpoints + 1; j++)
		{
			rm = 0.5f;
			mv = height[i][j] * (1.0f - rm) + rm*0.25f*(height[gp_wrap(i - 1)][j] + height[i][gp_wrap(j - 1)] + height[gp_wrap(i + 1)][j] + height[i][gp_wrap(j + 1)]);
			backterrain[i + terrain_gridpoints*j] = mv;
		}
	for (i = 0; i<terrain_gridpoints + 1; i++)
		for (j = 0; j<terrain_gridpoints + 1; j++)
		{
			height[i][j] = (backterrain[i + terrain_gridpoints*j]);
		}


	free(backterrain);

	//calculating normals
	for (i = 0; i<terrain_gridpoints + 1; i++)
		for (j = 0; j<terrain_gridpoints + 1; j++)
		{
			vec1.m128_f32[0] = 2 * terrain_geometry_scale;
			vec1.m128_f32[1] = terrain_geometry_scale*(height[gp_wrap(i + 1)][j] - height[gp_wrap(i - 1)][j]);
			vec1.m128_f32[2] = 0;
			vec2.m128_f32[0] = 0;
			vec2.m128_f32[1] = -terrain_geometry_scale*(height[i][gp_wrap(j + 1)] - height[i][gp_wrap(j - 1)]);
			vec2.m128_f32[2] = -2 * terrain_geometry_scale;

			XMStoreFloat3(&normal[i][j], XMVector3Normalize(XMVector3Cross(vec1, vec2)));
		}


	// buiding layerdef 
	byte* temp_layerdef_map_texture_pixels = (byte *)malloc(terrain_layerdef_map_texture_size*terrain_layerdef_map_texture_size * 4);
	byte* layerdef_map_texture_pixels = (byte *)malloc(terrain_layerdef_map_texture_size*terrain_layerdef_map_texture_size * 4);
	for (i = 0; i<terrain_layerdef_map_texture_size; i++)
		for (j = 0; j<terrain_layerdef_map_texture_size; j++)
		{
			x = (float)(terrain_gridpoints)*((float)i / (float)terrain_layerdef_map_texture_size);
			z = (float)(terrain_gridpoints)*((float)j / (float)terrain_layerdef_map_texture_size);
			ix = (int)floor(x);
			iz = (int)floor(z);
			rm = bilinear_interpolation(x - ix, z - iz, height[ix][iz], height[ix + 1][iz], height[ix + 1][iz + 1], height[ix][iz + 1])*terrain_geometry_scale;

			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 0;
			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 0;
			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 0;
			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 0;

			if ((rm>terrain_height_underwater_start) && (rm <= terrain_height_underwater_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 255;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 0;
			}

			if ((rm>terrain_height_sand_start) && (rm <= terrain_height_sand_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 255;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 0;
			}

			if ((rm>terrain_height_grass_start) && (rm <= terrain_height_grass_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 255;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 0;
			}

			mv = bilinear_interpolation(x - ix, z - iz, normal[ix][iz].y, normal[ix + 1][iz].y, normal[ix + 1][iz + 1].y, normal[ix][iz + 1].y);

			if ((mv<terrain_slope_grass_start) && (rm>terrain_height_sand_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 0;
			}

			if ((mv<terrain_slope_rocks_start) && (rm>terrain_height_rocks_start))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = 0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = 255;
			}

		}
	for (i = 0; i<terrain_layerdef_map_texture_size; i++)
		for (j = 0; j<terrain_layerdef_map_texture_size; j++)
		{
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3];
		}


	for (i = 2; i<terrain_layerdef_map_texture_size - 2; i++)
		for (j = 2; j<terrain_layerdef_map_texture_size - 2; j++)
		{
			int n1 = 0;
			int n2 = 0;
			int n3 = 0;
			int n4 = 0;
			for (k = -2; k<3; k++)
				for (l = -2; l<3; l++)
				{
					n1 += temp_layerdef_map_texture_pixels[((j + k)*terrain_layerdef_map_texture_size + i + l) * 4 + 0];
					n2 += temp_layerdef_map_texture_pixels[((j + k)*terrain_layerdef_map_texture_size + i + l) * 4 + 1];
					n3 += temp_layerdef_map_texture_pixels[((j + k)*terrain_layerdef_map_texture_size + i + l) * 4 + 2];
					n4 += temp_layerdef_map_texture_pixels[((j + k)*terrain_layerdef_map_texture_size + i + l) * 4 + 3];
				}
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 0] = (byte)(n1 / 25);
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 1] = (byte)(n2 / 25);
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 2] = (byte)(n3 / 25);
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size + i) * 4 + 3] = (byte)(n4 / 25);
		}

	// putting the generated data to textures

	subresource_data.pSysMem = layerdef_map_texture_pixels;
	subresource_data.SysMemPitch = terrain_layerdef_map_texture_size * 4;
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_layerdef_map_texture_size;
	tex_desc.Height = terrain_layerdef_map_texture_size;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	result = pDevice->CreateTexture2D(&tex_desc, &subresource_data, &layerdef_texture);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = tex_desc.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;
	pDevice->CreateShaderResourceView(layerdef_texture, &textureSRV_desc, &g_LayerdefTexture);

	//ID3D11Resource* tempResource;
	//ID3D11DeviceContext* pContext;
	//pDevice->GetImmediateContext(&pContext);
	//g_LayerdefTexture->GetResource(&tempResource);
	//SaveDDSTextureToFile(pContext, tempResource, L"g_LayerdefTexturee.DDS");

	free(temp_layerdef_map_texture_pixels);
	free(layerdef_map_texture_pixels);

	height_linear_array = new float[terrain_gridpoints*terrain_gridpoints * 4];
	patches_rawdata = new float[terrain_numpatches_1d*terrain_numpatches_1d * 4];

	for (int i = 0; i<terrain_gridpoints; i++)
		for (int j = 0; j<terrain_gridpoints; j++)
		{
			height_linear_array[(i + j*terrain_gridpoints) * 4 + 0] = normal[i][j].x;
			height_linear_array[(i + j*terrain_gridpoints) * 4 + 1] = normal[i][j].y;
			height_linear_array[(i + j*terrain_gridpoints) * 4 + 2] = normal[i][j].z;
			height_linear_array[(i + j*terrain_gridpoints) * 4 + 3] = height[i][j];
		}
	subresource_data.pSysMem = height_linear_array;
	subresource_data.SysMemPitch = terrain_gridpoints * 4 * sizeof(float);
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_gridpoints;
	tex_desc.Height = terrain_gridpoints;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	result = pDevice->CreateTexture2D(&tex_desc, &subresource_data, &heightmap_texture);

	free(height_linear_array);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = tex_desc.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	pDevice->CreateShaderResourceView(heightmap_texture, &textureSRV_desc, &g_HeightfieldTexture);

	//ID3D11Resource* tempResource;
	//ID3D11DeviceContext* pContext;
	//pDevice->GetImmediateContext(&pContext);
	//g_HeightfieldTexture->GetResource(&tempResource);
	//SaveDDSTextureToFile(pContext, tempResource, L"heightmap_texture.DDS");

	//building depthmap
	byte * depth_shadow_map_texture_pixels = (byte *)malloc(terrain_depth_shadow_map_texture_size*terrain_depth_shadow_map_texture_size * 4);
	for (i = 0; i<terrain_depth_shadow_map_texture_size; i++)
		for (j = 0; j<terrain_depth_shadow_map_texture_size; j++)
		{
			x = (float)(terrain_gridpoints)*((float)i / (float)terrain_depth_shadow_map_texture_size);
			z = (float)(terrain_gridpoints)*((float)j / (float)terrain_depth_shadow_map_texture_size);
			ix = (int)floor(x);
			iz = (int)floor(z);
			rm = bilinear_interpolation(x - ix, z - iz, height[ix][iz], height[ix + 1][iz], height[ix + 1][iz + 1], height[ix][iz + 1])*terrain_geometry_scale;

			if (rm>0)
			{
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 0] = 0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 1] = 0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 2] = 0;
			}
			else
			{
				float no = (1.0f*255.0f*(rm / (terrain_minheight*terrain_geometry_scale))) - 1.0f;
				if (no>255) no = 255;
				if (no<0) no = 0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 0] = (byte)no;

				no = (10.0f*255.0f*(rm / (terrain_minheight*terrain_geometry_scale))) - 40.0f;
				if (no>255) no = 255;
				if (no<0) no = 0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 1] = (byte)no;

				no = (100.0f*255.0f*(rm / (terrain_minheight*terrain_geometry_scale))) - 300.0f;
				if (no>255) no = 255;
				if (no<0) no = 0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 2] = (byte)no;
			}
			depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size + i) * 4 + 3] = 0;
		}

	subresource_data.pSysMem = depth_shadow_map_texture_pixels;
	subresource_data.SysMemPitch = terrain_depth_shadow_map_texture_size * 4;
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_depth_shadow_map_texture_size;
	tex_desc.Height = terrain_depth_shadow_map_texture_size;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	result = pDevice->CreateTexture2D(&tex_desc, &subresource_data, &depthmap_texture);

	ZeroMemory(&textureSRV_desc, sizeof(textureSRV_desc));
	textureSRV_desc.Format = tex_desc.Format;
	textureSRV_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	pDevice->CreateShaderResourceView(depthmap_texture, &textureSRV_desc, &g_DepthMapTexture);

	free(depth_shadow_map_texture_pixels);

	// creating terrain vertex buffer
	for (int i = 0; i<terrain_numpatches_1d; i++)
		for (int j = 0; j<terrain_numpatches_1d; j++)
		{
			patches_rawdata[(i + j*terrain_numpatches_1d) * 4 + 0] = i*terrain_geometry_scale*terrain_gridpoints / terrain_numpatches_1d;
			patches_rawdata[(i + j*terrain_numpatches_1d) * 4 + 1] = j*terrain_geometry_scale*terrain_gridpoints / terrain_numpatches_1d;
			patches_rawdata[(i + j*terrain_numpatches_1d) * 4 + 2] = terrain_geometry_scale*terrain_gridpoints / terrain_numpatches_1d;
			patches_rawdata[(i + j*terrain_numpatches_1d) * 4 + 3] = terrain_geometry_scale*terrain_gridpoints / terrain_numpatches_1d;
		}

	D3D11_BUFFER_DESC buf_desc;
	memset(&buf_desc, 0, sizeof(buf_desc));

	buf_desc.ByteWidth = terrain_numpatches_1d*terrain_numpatches_1d * 4 * sizeof(float);
	buf_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buf_desc.Usage = D3D11_USAGE_DEFAULT;

	//printf("%f %f %f %f\n", patches_rawdata[0], patches_rawdata[1], patches_rawdata[2], patches_rawdata[3]);

	subresource_data.pSysMem = patches_rawdata;
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;

	result = pDevice->CreateBuffer(&buf_desc, &subresource_data, &heightfield_vertexbuffer);
	free(patches_rawdata);

	// creating sky vertex buffer
	float *sky_vertexdata;
	int floatnum;
	sky_vertexdata = new float[sky_gridpoints*(sky_gridpoints + 2) * 2 * 6];

	for (int j = 0; j<sky_gridpoints; j++)
	{

		i = 0;
		floatnum = (j*(sky_gridpoints + 2) * 2) * 6;
		sky_vertexdata[floatnum + 0] = terrain_gridpoints*terrain_geometry_scale*0.5f + 4000.0f*cos(2.0f*PI*(float)i / (float)sky_gridpoints)*cos(-0.5f*PI + PI*(float)j / (float)sky_gridpoints);
		sky_vertexdata[floatnum + 1] = 4000.0f*sin(-0.5f*PI + PI*(float)(j) / (float)sky_gridpoints);
		sky_vertexdata[floatnum + 2] = terrain_gridpoints*terrain_geometry_scale*0.5f + 4000.0f*sin(2.0f*PI*(float)i / (float)sky_gridpoints)*cos(-0.5f*PI + PI*(float)j / (float)sky_gridpoints);
		sky_vertexdata[floatnum + 3] = 1;
		sky_vertexdata[floatnum + 4] = (sky_texture_angle + (float)i / (float)sky_gridpoints);
		sky_vertexdata[floatnum + 5] = 2.0f - 2.0f*(float)j / (float)sky_gridpoints;
		floatnum += 6;
		for (i = 0; i<sky_gridpoints + 1; i++)
		{
			sky_vertexdata[floatnum + 0] = terrain_gridpoints*terrain_geometry_scale*0.5f + 4000.0f*cos(2.0f*PI*(float)i / (float)sky_gridpoints)*cos(-0.5f*PI + PI*(float)j / (float)sky_gridpoints);
			sky_vertexdata[floatnum + 1] = 4000.0f*sin(-0.5f*PI + PI*(float)(j) / (float)sky_gridpoints);
			sky_vertexdata[floatnum + 2] = terrain_gridpoints*terrain_geometry_scale*0.5f + 4000.0f*sin(2.0f*PI*(float)i / (float)sky_gridpoints)*cos(-0.5f*PI + PI*(float)j / (float)sky_gridpoints);
			sky_vertexdata[floatnum + 3] = 1;
			sky_vertexdata[floatnum + 4] = (sky_texture_angle + (float)i / (float)sky_gridpoints);
			sky_vertexdata[floatnum + 5] = 2.0f - 2.0f*(float)j / (float)sky_gridpoints;
			floatnum += 6;
			sky_vertexdata[floatnum + 0] = terrain_gridpoints*terrain_geometry_scale*0.5f + 4000.0f*cos(2.0f*PI*(float)i / (float)sky_gridpoints)*cos(-0.5f*PI + PI*(float)(j + 1) / (float)sky_gridpoints);
			sky_vertexdata[floatnum + 1] = 4000.0f*sin(-0.5f*PI + PI*(float)(j + 1) / (float)sky_gridpoints);
			sky_vertexdata[floatnum + 2] = terrain_gridpoints*terrain_geometry_scale*0.5f + 4000.0f*sin(2.0f*PI*(float)i / (float)sky_gridpoints)*cos(-0.5f*PI + PI*(float)(j + 1) / (float)sky_gridpoints);
			sky_vertexdata[floatnum + 3] = 1;
			sky_vertexdata[floatnum + 4] = (sky_texture_angle + (float)i / (float)sky_gridpoints);
			sky_vertexdata[floatnum + 5] = 2.0f - 2.0f*(float)(j + 1) / (float)sky_gridpoints;
			floatnum += 6;
		}
		i = sky_gridpoints;
		sky_vertexdata[floatnum + 0] = terrain_gridpoints*terrain_geometry_scale*0.5f + 4000.0f*cos(2.0f*PI*(float)i / (float)sky_gridpoints)*cos(-0.5f*PI + PI*(float)(j + 1) / (float)sky_gridpoints);
		sky_vertexdata[floatnum + 1] = 4000.0f*sin(-0.5f*PI + PI*(float)(j + 1) / (float)sky_gridpoints);
		sky_vertexdata[floatnum + 2] = terrain_gridpoints*terrain_geometry_scale*0.5f + 4000.0f*sin(2.0f*PI*(float)i / (float)sky_gridpoints)*cos(-0.5f*PI + PI*(float)(j + 1) / (float)sky_gridpoints);
		sky_vertexdata[floatnum + 3] = 1;
		sky_vertexdata[floatnum + 4] = (sky_texture_angle + (float)i / (float)sky_gridpoints);
		sky_vertexdata[floatnum + 5] = 2.0f - 2.0f*(float)(j + 1) / (float)sky_gridpoints;
		floatnum += 6;
	}

	memset(&buf_desc, 0, sizeof(buf_desc));

	buf_desc.ByteWidth = sky_gridpoints*(sky_gridpoints + 2) * 2 * 6 * sizeof(float);
	buf_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buf_desc.Usage = D3D11_USAGE_DEFAULT;

	subresource_data.pSysMem = sky_vertexdata;
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;

	result = pDevice->CreateBuffer(&buf_desc, &subresource_data, &sky_vertexbuffer);

	free(sky_vertexdata);
}

void Terrain::LoadTextures()
{
	HRESULT hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/rock_bump6.dds", nullptr, &g_RockBumpTexture, 0, nullptr);

	hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/terrain_rock4.dds", nullptr, &g_RockDiffuseTexture, 0, nullptr);

	hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/sand_diffuse.dds", nullptr, &g_SandDiffuseTexture, 0, nullptr);

	hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/rock_bump4.dds", nullptr, &g_SandBumpTexture, 0, nullptr);

	hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/terrain_grass.dds", nullptr, &g_GrassDiffuseTexture, 0, nullptr);

	hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/terrain_slope.dds", nullptr, &g_SlopeDiffuseTexture, 0, nullptr);

	hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/lichen1_normal.dds", nullptr, &g_SandMicroBumpTexture, 0, nullptr);

	hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/rock_bump4.dds", nullptr, &g_RockMicroBumpTexture, 0, nullptr);

	hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/water_bump.dds", nullptr, &g_WaterBumpTexture, 0, nullptr);

	hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/sky.dds", nullptr, &g_SkyTexture, 0, nullptr);

	//hr = CreateDDSTextureFromFile(pDevice, L"TerrainTextures/refraction_depth_resourceSRV.DDS", nullptr, &g_SkyTexture, 0, nullptr);
}

void Terrain::Render(Camera *cam, ID3D11RenderTargetView *colorBuffer, ID3D11DepthStencilView  *backBuffer, double time)
{
	ID3D11DeviceContext* pContext;
	pDevice->GetImmediateContext(&pContext);

	//saving scene color buffer and back buffer to constants
	UINT cRT = 1;
	D3D11_VIEWPORT currentViewport;
	pContext->RSGetViewports(&cRT, &currentViewport);
	pContext->OMGetRenderTargets(1, &colorBuffer, &backBuffer);

	D3D11_VIEWPORT main_Viewport;
	main_Viewport.Width = (float)BackbufferWidth*main_buffer_size_multiplier;
	main_Viewport.Height = (float)BackbufferHeight*main_buffer_size_multiplier;
	main_Viewport.MaxDepth = 1;
	main_Viewport.MinDepth = 0;
	main_Viewport.TopLeftX = 0;
	main_Viewport.TopLeftY = 0;

	pContext->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	pContext->HSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	pContext->DSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	pContext->PSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	renderTarrainToDepthBuffer(pContext, cam);

	bool g_RenderCaustics = true;
	if (g_RenderCaustics)
	{
		renderCaustics(pContext, cam, time);
	}

	renderReflection(pContext, cam);

	renderTarrain(pContext, main_Viewport, cam);

	renderRefraction(pContext, main_Viewport);

	renderWater(pContext, main_Viewport, cam, time);

	//renderSky(pContext);

	renderToBackBuffer(pContext, colorBuffer, backBuffer, currentViewport);

	SAFE_RELEASE(colorBuffer);
	SAFE_RELEASE(backBuffer);

	pContext->Release();
}

void Terrain::renderTarrainToDepthBuffer(ID3D11DeviceContext* pContext, Camera *cam)
{
	D3D11_VIEWPORT shadowmap_resource_viewport;
	shadowmap_resource_viewport.Width = shadowmap_resource_buffer_size_xy;
	shadowmap_resource_viewport.Height = shadowmap_resource_buffer_size_xy;
	shadowmap_resource_viewport.MaxDepth = 1;
	shadowmap_resource_viewport.MinDepth = 0;
	shadowmap_resource_viewport.TopLeftX = 0;
	shadowmap_resource_viewport.TopLeftY = 0;

	pContext->RSSetViewports(1, &shadowmap_resource_viewport);

	pContext->RSSetState(pCullBackMS);
	pContext->OMSetDepthStencilState(pDepthNormal, 0);

	const float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pContext->OMSetBlendState(pNoBlending, BlendFactor, 0xFFFFFFFF);

	pContext->OMSetRenderTargets(0, NULL, shadowmap_resourceDSV);
	pContext->ClearDepthStencilView(shadowmap_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	pContext->IASetInputLayout(heightfield_inputlayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	pContext->HSSetShaderResources(0, 1, &g_HeightfieldTexture);

	pContext->DSSetShaderResources(0, 1, &g_HeightfieldTexture);
	pContext->DSSetShaderResources(1, 1, &g_LayerdefTexture);
	pContext->DSSetShaderResources(2, 1, &g_SandBumpTexture);
	pContext->DSSetShaderResources(3, 1, &g_RockBumpTexture);

	pContext->VSSetShader(PassThroughVS, nullptr, 0);
	pContext->HSSetShader(PatchHS, nullptr, 0);
	pContext->DSSetShader(HeightFieldPatchDS, nullptr, 0);
	pContext->PSSetShader(ColorPS, nullptr, 0);

	UINT stride = sizeof(float) * 4;//PATCH_PARAMETERS DXGI_FORMAT_R32G32B32A32_FLOAT
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);

	SetupLightView(cam);

	cbuffer.g_TerrainBeingRendered = 1.0f;
	cbuffer.g_SkipCausticsCalculation = 1.0f;
	cbuffer.g_LightPosition = XMFLOAT3(-10000.0f, 6500.0f, 10000.0f);
	cbuffer.g_DynamicTessFactor = 50.0f;
	cbuffer.g_StaticTessFactor = 12.0f;
	cbuffer.g_UseDynamicLOD = 1.0f;
	cbuffer.g_RenderCaustics = 1.0f;
	cbuffer.g_HeightFieldSize = terrain_gridpoints*terrain_geometry_scale;
	cbuffer.g_FrustumCullInHS = 1.0f;

	pContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbuffer, 0, 0);

	unsigned int vertexCount = terrain_numpatches_1d*terrain_numpatches_1d;//4096
	pContext->Draw(vertexCount, 0);

	//чистка
	pContext->OMSetRenderTargets(0, NULL, NULL);

	pContext->VSSetShader(nullptr, nullptr, 0);
	pContext->HSSetShader(nullptr, nullptr, 0);
	pContext->DSSetShader(nullptr, nullptr, 0);
	pContext->PSSetShader(nullptr, nullptr, 0);
}
void Terrain::renderCaustics(ID3D11DeviceContext* pContext, Camera *cam, double time)
{
	D3D11_VIEWPORT water_normalmap_resource_viewport;
	water_normalmap_resource_viewport.Width = water_normalmap_resource_buffer_size_xy;
	water_normalmap_resource_viewport.Height = water_normalmap_resource_buffer_size_xy;
	water_normalmap_resource_viewport.MaxDepth = 1;
	water_normalmap_resource_viewport.MinDepth = 0;
	water_normalmap_resource_viewport.TopLeftX = 0;
	water_normalmap_resource_viewport.TopLeftY = 0;

	pContext->RSSetViewports(1, &water_normalmap_resource_viewport);

	pContext->RSSetState(pNoCullMS);
	pContext->OMSetDepthStencilState(pNoDepthStencil, 0);
	const float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pContext->OMSetBlendState(pNoBlending, BlendFactor, 0xFFFFFFFF);

	pContext->OMSetRenderTargets(1, &water_normalmap_resourceRTV, NULL);

	float ClearColor[4] = { 0.8f, 0.8f, 1.0f, 1.0f };
	pContext->ClearRenderTargetView(water_normalmap_resourceRTV, ClearColor);

	pContext->IASetInputLayout(trianglestrip_inputlayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	pContext->VSSetShader(WaterNormalmapCombineVS, nullptr, 0);
	pContext->HSSetShader(nullptr, nullptr, 0);
	pContext->DSSetShader(nullptr, nullptr, 0);
	pContext->PSSetShader(WaterNormalmapCombinePS, nullptr, 0);

	UINT stride = sizeof(float) * 6;//POSITION (DXGI_FORMAT_R32G32B32A32_FLOAT) + TEXCOORD (DXGI_FORMAT_R32G32_FLOAT)
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);

	SetupNormalView(cam);

	cbuffer.g_WaterBumpTexcoordShift = XMFLOAT2(time*1.5f, time*0.75f);//XMFLOAT2(0, 0);
	cbuffer.g_HeightFieldSize = terrain_gridpoints*terrain_geometry_scale;

	pContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbuffer, 0, 0);

	pContext->Draw(4, 0); // just need to pass 4 vertices to shader

	//чистка
	pContext->OMSetRenderTargets(0, NULL, NULL);

	pContext->VSSetShader(nullptr, nullptr, 0);
	pContext->HSSetShader(nullptr, nullptr, 0);
	pContext->DSSetShader(nullptr, nullptr, 0);
	pContext->PSSetShader(nullptr, nullptr, 0);	
}
void Terrain::renderReflection(ID3D11DeviceContext* pContext, Camera *cam)
{
	D3D11_VIEWPORT reflection_Viewport;
	reflection_Viewport.Width = (float)BackbufferWidth*reflection_buffer_size_multiplier;
	reflection_Viewport.Height = (float)BackbufferHeight*reflection_buffer_size_multiplier;
	reflection_Viewport.MaxDepth = 1;
	reflection_Viewport.MinDepth = 0;
	reflection_Viewport.TopLeftX = 0;
	reflection_Viewport.TopLeftY = 0;

	pContext->RSSetViewports(1, &reflection_Viewport);

	pContext->OMSetRenderTargets(1, &reflection_color_resourceRTV, reflection_depth_resourceDSV);

	float RefractionClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	pContext->ClearRenderTargetView(reflection_color_resourceRTV, RefractionClearColor);
	pContext->ClearDepthStencilView(reflection_depth_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	SetupReflectionView(cam);
	//
	// drawing sky to reflection RT

	//pContext->VSSetShader(SkyVS, nullptr, 0);
	//pContext->PSSetShader(SkyPS, nullptr, 0);

	//pContext->IASetInputLayout(trianglestrip_inputlayout);
	//pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	UINT stride = sizeof(float) * 6;//POSITION (DXGI_FORMAT_R32G32B32A32_FLOAT) + TEXCOORD (DXGI_FORMAT_R32G32_FLOAT)
	UINT offset = 0;
	//pContext->IASetVertexBuffers(0, 1, &sky_vertexbuffer, &stride, &offset);

	//pContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbuffer, 0, 0);

	//pContext->Draw(sky_gridpoints*(sky_gridpoints + 2) * 2, 0);

	pContext->RSSetState(pCullBackMS);
	pContext->OMSetDepthStencilState(pDepthNormal, 0);
	const float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pContext->OMSetBlendState(pNoBlending, BlendFactor, 0xFFFFFFFF);

	pContext->IASetInputLayout(heightfield_inputlayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	pContext->HSSetShaderResources(0, 1, &g_HeightfieldTexture);

	pContext->DSSetShaderResources(0, 1, &g_HeightfieldTexture);
	pContext->DSSetShaderResources(1, 1, &g_LayerdefTexture);
	pContext->DSSetShaderResources(2, 1, &g_SandBumpTexture);
	pContext->DSSetShaderResources(3, 1, &g_RockBumpTexture);

	pContext->PSSetShaderResources(0, 1, &g_SandMicroBumpTexture);
	pContext->PSSetShaderResources(1, 1, &g_RockMicroBumpTexture);
	pContext->PSSetShaderResources(2, 1, &g_SlopeDiffuseTexture);
	pContext->PSSetShaderResources(3, 1, &g_SandDiffuseTexture);
	pContext->PSSetShaderResources(4, 1, &g_RockDiffuseTexture);
	pContext->PSSetShaderResources(5, 1, &g_GrassDiffuseTexture);
	pContext->PSSetShaderResources(6, 1, &g_DepthTexture);

	pContext->VSSetShader(PassThroughVS, nullptr, 0);
	pContext->HSSetShader(PatchHS, nullptr, 0);
	pContext->DSSetShader(HeightFieldPatchDS, nullptr, 0);
	pContext->PSSetShader(HeightFieldPatchPS, nullptr, 0);

	stride = sizeof(float) * 4;//PATCH_PARAMETERS DXGI_FORMAT_R32G32B32A32_FLOAT
	pContext->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);

	SetupLightView2(cam);
	cbuffer.g_SkipCausticsCalculation = 1.0f;
	cbuffer.g_LightPosition = XMFLOAT3(-10000.0f, 6500.0f, 10000.0f);
	cbuffer.g_DynamicTessFactor = 50.0f;
	cbuffer.g_StaticTessFactor = 12.0f;
	cbuffer.g_UseDynamicLOD = 1.0f;
	cbuffer.g_RenderCaustics = 1.0f;
	cbuffer.g_HeightFieldSize = terrain_gridpoints*terrain_geometry_scale;
	cbuffer.g_FrustumCullInHS = 1.0f;

	pContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbuffer, 0, 0);

	unsigned int vertexCount = terrain_numpatches_1d*terrain_numpatches_1d;//4096
	pContext->Draw(vertexCount, 0);

	// чистка
	pContext->OMSetRenderTargets(0, NULL, NULL);
}
void Terrain::renderTarrain(ID3D11DeviceContext* pContext, D3D11_VIEWPORT &main_Viewport, Camera *cam)
{
	pContext->RSSetViewports(1, &main_Viewport);

	pContext->RSSetState(pCullBackMS);
	pContext->OMSetDepthStencilState(pDepthNormal, 0);

	const float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pContext->OMSetBlendState(pNoBlending, BlendFactor, 0xFFFFFFFF);

	pContext->HSSetShaderResources(0, 1, &g_HeightfieldTexture);

	pContext->DSSetShaderResources(0, 1, &g_HeightfieldTexture);
	pContext->DSSetShaderResources(1, 1, &g_LayerdefTexture);
	pContext->DSSetShaderResources(2, 1, &g_SandBumpTexture);
	pContext->DSSetShaderResources(3, 1, &g_RockBumpTexture);
	pContext->DSSetShaderResources(4, 1, &g_WaterNormalMapTexture);//

	pContext->PSSetShaderResources(0, 1, &g_SandMicroBumpTexture);
	pContext->PSSetShaderResources(1, 1, &g_RockMicroBumpTexture);
	pContext->PSSetShaderResources(2, 1, &g_SlopeDiffuseTexture);
	pContext->PSSetShaderResources(3, 1, &g_SandDiffuseTexture);
	pContext->PSSetShaderResources(4, 1, &g_RockDiffuseTexture);
	pContext->PSSetShaderResources(5, 1, &g_GrassDiffuseTexture);
	pContext->PSSetShaderResources(6, 1, &g_DepthTexture);

	pContext->OMSetRenderTargets(1, &main_color_resourceRTV, main_depth_resourceDSV);

	float ClearColor[4] = { 0.8f, 0.8f, 1.0f, 1.0f };
	pContext->ClearRenderTargetView(main_color_resourceRTV, ClearColor);
	pContext->ClearDepthStencilView(main_depth_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	SetupLightView2(cam);
	SetupNormalView(cam);

	pContext->IASetInputLayout(heightfield_inputlayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);

	pContext->VSSetShader(PassThroughVS, nullptr, 0);
	pContext->HSSetShader(PatchHS, nullptr, 0);
	pContext->DSSetShader(HeightFieldPatchDS, nullptr, 0);
	pContext->PSSetShader(HeightFieldPatchPS, nullptr, 0);

	UINT stride = stride = sizeof(float) * 4;//PATCH_PARAMETERS DXGI_FORMAT_R32G32B32A32_FLOAT
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);

	cbuffer.g_TerrainBeingRendered = 1.0f;
	cbuffer.g_SkipCausticsCalculation = 0.0f;
	cbuffer.g_LightPosition = XMFLOAT3(-10000.0f, 6500.0f, 10000.0f);
	cbuffer.g_DynamicTessFactor = 50.0f;
	cbuffer.g_StaticTessFactor = 12.0f;
	cbuffer.g_UseDynamicLOD = 1.0f;
	cbuffer.g_RenderCaustics = 1.0f;
	cbuffer.g_HeightFieldSize = terrain_gridpoints*terrain_geometry_scale;
	cbuffer.g_FrustumCullInHS = 1.0f;

	pContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbuffer, 0, 0);

	pContext->Draw(terrain_numpatches_1d*terrain_numpatches_1d, 0);

	//чистка
	pContext->OMSetRenderTargets(0, NULL, NULL);
	pContext->DSSetShaderResources(4, 1, &nullSRV);

	pContext->PSSetShaderResources(6, 1, &nullSRV);
}
void Terrain::renderRefraction(ID3D11DeviceContext* pContext, D3D11_VIEWPORT &main_Viewport)
{
	pContext->RSSetViewports(1, &main_Viewport);

	pContext->RSSetState(pNoCullMS);
	pContext->OMSetDepthStencilState(pNoDepthStencil, 0);
	const float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pContext->OMSetBlendState(pNoBlending, BlendFactor, 0xFFFFFFFF);

	pContext->ResolveSubresource(refraction_color_resource, 0, main_color_resource, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	pContext->PSSetShaderResources(13, 1, &main_depth_resourceSRV);

	pContext->OMSetRenderTargets(1, &refraction_depth_resourceRTV, NULL);

	pContext->IASetInputLayout(trianglestrip_inputlayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	pContext->VSSetShader(FullScreenQuadVS, nullptr, 0);
	pContext->HSSetShader(nullptr, nullptr, 0);
	pContext->DSSetShader(nullptr, nullptr, 0);
	pContext->PSSetShader(RefractionDepthManualResolveMS1PS, nullptr, 0);

	UINT stride = sizeof(float) * 6;//POSITION (DXGI_FORMAT_R32G32B32A32_FLOAT) + TEXCOORD (DXGI_FORMAT_R32G32_FLOAT)
	UINT offset = 0;

	pContext->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);

	pContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbuffer, 0, 0);

	pContext->Draw(4, 0); // just need to pass 4 vertices to shader

	//чистка
	pContext->PSSetShaderResources(13, 1, &nullSRV);
}
void Terrain::renderWater(ID3D11DeviceContext* pContext, D3D11_VIEWPORT &main_Viewport, Camera *cam, double time)
{
	pContext->RSSetViewports(1, &main_Viewport);

	pContext->RSSetState(pCullBackMS);
	pContext->OMSetDepthStencilState(pDepthNormal, 0);

	const float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pContext->OMSetBlendState(pNoBlending, BlendFactor, 0xFFFFFFFF);

	pContext->OMSetRenderTargets(1, &main_color_resourceRTV, main_depth_resourceDSV);

	pContext->HSSetShaderResources(0, 1, &g_HeightfieldTexture);

	pContext->DSSetShaderResources(4, 1, &g_WaterNormalMapTexture);
	pContext->DSSetShaderResources(5, 1, &g_DepthMapTexture);
	pContext->DSSetShaderResources(6, 1, &g_WaterBumpTexture);

	pContext->PSSetShaderResources(6, 1, &g_DepthTexture);
	pContext->PSSetShaderResources(7, 1, &g_WaterBumpTexture);
	pContext->PSSetShaderResources(9, 1, &g_RefractionDepthTextureResolved);
	pContext->PSSetShaderResources(10, 1, &g_ReflectionTexture);
	pContext->PSSetShaderResources(11, 1, &g_RefractionTexture);

	pContext->VSSetShader(PassThroughVS, nullptr, 0);
	pContext->HSSetShader(PatchHS, nullptr, 0);
	pContext->DSSetShader(WaterPatchDS, nullptr, 0);
	pContext->PSSetShader(WaterPatchPS, nullptr, 0);

	pContext->IASetInputLayout(heightfield_inputlayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
	
	UINT stride = sizeof(float) * 4;//PATCH_PARAMETERS DXGI_FORMAT_R32G32B32A32_FLOAT
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &heightfield_vertexbuffer, &stride, &offset);

	SetupLightView2(cam);

	cbuffer.g_ZNear = 1.0f;
	cbuffer.g_ZFar = 25000.0f;
	cbuffer.g_TerrainBeingRendered = 0.0f;
	cbuffer.g_LightPosition = XMFLOAT3(-10000.0f, 6500.0f, 10000.0f);
	cbuffer.g_WaterBumpTexcoordShift = XMFLOAT2(time*1.5f, time*0.75f);//XMFLOAT2(0, 0); 
	cbuffer.g_ScreenSizeInv = XMFLOAT2(0.000710, 0.001319);
	cbuffer.g_DynamicTessFactor = 50.0f;
	cbuffer.g_StaticTessFactor = 12.0f;
	cbuffer.g_UseDynamicLOD = 1.0f;
	cbuffer.g_HeightFieldSize = terrain_gridpoints*terrain_geometry_scale;
	cbuffer.g_FrustumCullInHS = 1.0f;

	pContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbuffer, 0, 0);

	pContext->Draw(terrain_numpatches_1d*terrain_numpatches_1d, 0);

	//чистка
	pContext->DSSetShaderResources(4, 1, &nullSRV);//

	pContext->PSSetShaderResources(6, 1, &nullSRV);//
	pContext->PSSetShaderResources(9, 1, &nullSRV);//
	pContext->PSSetShaderResources(10, 1, &nullSRV);//
	pContext->PSSetShaderResources(11, 1, &nullSRV);//
}
void Terrain::renderToBackBuffer(ID3D11DeviceContext* pContext, ID3D11RenderTargetView *colorBuffer, ID3D11DepthStencilView  *backBuffer, D3D11_VIEWPORT &currentViewport)
{
	pContext->RSSetViewports(1, &currentViewport);

	pContext->RSSetState(m_states->CullNone());
	pContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
	const float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pContext->OMSetBlendState(pNoBlending, BlendFactor, 0xFFFFFFFF);

	pContext->OMSetRenderTargets(1, &colorBuffer, backBuffer);

	pContext->ResolveSubresource(main_color_resource_resolved, 0, main_color_resource, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	pContext->PSSetShaderResources(12, 1, &g_MainTexture);

	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	pContext->VSSetShader(FullScreenQuadVS, nullptr, 0);
	pContext->HSSetShader(nullptr, nullptr, 0);
	pContext->DSSetShader(nullptr, nullptr, 0);
	pContext->PSSetShader(MainToBackBufferPS, nullptr, 0);

	cbuffer.g_MainBufferSizeMultiplier = main_buffer_size_multiplier;

	pContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbuffer, 0, 0);

	pContext->Draw(4, 0); // just need to pass 4 vertices to shader

	//чистка
	pContext->PSSetShaderResources(12, 1, &nullSRV);//

	pContext->VSSetShader(nullptr, nullptr, 0);
	pContext->HSSetShader(nullptr, nullptr, 0);
	pContext->DSSetShader(nullptr, nullptr, 0);
	pContext->PSSetShader(nullptr, nullptr, 0);
}
void Terrain::renderSky(ID3D11DeviceContext* pContext)
{
	pContext->PSSetShaderResources(8, 1, &g_SkyTexture);

	pContext->VSSetShader(SkyVS, nullptr, 0);
	pContext->PSSetShader(SkyPS, nullptr, 0);

	pContext->IASetInputLayout(trianglestrip_inputlayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT stride = sizeof(float) * 6;//POSITION (DXGI_FORMAT_R32G32B32A32_FLOAT) + TEXCOORD (DXGI_FORMAT_R32G32_FLOAT)
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &sky_vertexbuffer, &stride, &offset);

	cbuffer.g_LightPosition = XMFLOAT3(-10000.0f, 6500.0f, 10000.0f);
	pContext->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbuffer, 0, 0);

	pContext->Draw(sky_gridpoints*(sky_gridpoints + 2) * 2, 0);
}

void Terrain::SetupLightView(Camera *cam)
{
	XMVECTOR EyePoint = XMVectorSet(-10000.0f, 6500.0f, 10000.0f, 0);
	XMVECTOR LookAtPoint = XMVectorSet(terrain_far_range / 2.0f, 0.0f, terrain_far_range / 2.0f, 0);
	XMVECTOR lookUp = XMVectorSet(0, 1, 0, 0);

	XMVECTOR vectLength = XMVector3Length(EyePoint);

	float nr = vectLength.m128_f32[0] - terrain_far_range*0.7f;
	float fr = vectLength.m128_f32[0] + terrain_far_range*0.7f;

	XMMATRIX mView = XMMatrixLookAtLH(EyePoint, LookAtPoint, lookUp);
	XMMATRIX mProjMatrix = XMMatrixOrthographicLH(724, terrain_far_range, nr, fr);
	XMMATRIX mViewProj = mView * mProjMatrix;
	//XMMATRIX mViewProjInv = XMMatrixInverse(NULL, mViewProj);

	XMVECTOR direction = XMLoadFloat3(&cam->GetLookAtPt()) - XMLoadFloat3(&cam->GetEyePt());
	XMVECTOR normalized_direction = XMVector3Normalize(direction);
	XMVECTOR cameraPosition = XMLoadFloat3(&cam->GetEyePt());

	XMStoreFloat4x4(&cbuffer.g_ModelViewProjectionMatrix, XMMatrixTranspose(mViewProj));

	XMStoreFloat3(&cbuffer.g_CameraPosition, cameraPosition);
	XMStoreFloat3(&cbuffer.g_CameraDirection, normalized_direction);

	cbuffer.g_HalfSpaceCullSign = 1.0;//?
	cbuffer.g_HalfSpaceCullPosition = terrain_minheight * 2;//?
}

void Terrain::SetupLightView2(Camera *cam)
{
	XMVECTOR EyePoint = XMVectorSet(-10000.0f, 6500.0f, 10000.0f, 0);
	XMVECTOR LookAtPoint = XMVectorSet(terrain_far_range / 2.0f, 0.0f, terrain_far_range / 2.0f, 0);
	XMVECTOR lookUp = XMVectorSet(0, 1, 0, 0);

	XMVECTOR vectLength = XMVector3Length(EyePoint);

	float nr = vectLength.m128_f32[0] - terrain_far_range*0.7f;
	float fr = vectLength.m128_f32[0] + terrain_far_range*0.7f;

	XMMATRIX mView = XMMatrixLookAtLH(EyePoint, LookAtPoint, lookUp);
	XMMATRIX mProjMatrix = XMMatrixOrthographicLH(724, terrain_far_range, nr, fr);
	XMMATRIX mViewProj = mView * mProjMatrix;
	//XMMATRIX mViewProjInv = XMMatrixInverse(NULL, mViewProj);

	XMStoreFloat4x4(&cbuffer.g_LightModelViewProjectionMatrix, XMMatrixTranspose(mViewProj));//?
																							 //XMStoreFloat4x4(&cbuffer.g_LightModelViewProjectionMatrixInv, XMMatrixTranspose(mViewProjInv));//?
}

void Terrain::SetupNormalView(Camera *cam)
{
	XMVECTOR EyePoint = XMLoadFloat3(&cam->GetEyePt());
	XMVECTOR LookAtPoint = XMLoadFloat3(&cam->GetLookAtPt());

	XMMATRIX mView = XMLoadFloat4x4(&cam->GetViewMatrix());

	XMMATRIX mProjMatrix = XMLoadFloat4x4(&cam->GetProjMatrix());
	XMMATRIX mViewProj = mView * mProjMatrix;
	XMMATRIX mViewProjInv = XMMatrixInverse(NULL, mViewProj);
	XMVECTOR cameraPosition = XMLoadFloat3(&cam->GetEyePt());

	XMStoreFloat4x4(&cbuffer.g_ModelViewMatrix, XMMatrixTranspose(mView));
	XMStoreFloat4x4(&cbuffer.g_ModelViewProjectionMatrix, XMMatrixTranspose(mViewProj));
	XMStoreFloat4x4(&cbuffer.g_ModelViewProjectionMatrixInv, XMMatrixTranspose(mViewProjInv));

	XMStoreFloat3(&cbuffer.g_CameraPosition, cameraPosition);

	XMVECTOR direction = LookAtPoint - EyePoint;
	XMVECTOR normalized_direction = XMVector3Normalize(direction);

	XMStoreFloat3(&cbuffer.g_CameraDirection, normalized_direction);

	cbuffer.g_HalfSpaceCullSign = 1.0;
	cbuffer.g_HalfSpaceCullPosition = terrain_minheight * 2;
}

void Terrain::SetupReflectionView(Camera *cam)
{
	float aspectRatio = BackbufferWidth / BackbufferHeight;
	//printf("%f %f\n", BackbufferWidth, BackbufferHeight);

	const XMFLOAT3 EyePointO = cam->GetEyePt();
	const XMFLOAT3 LookAtPointO = cam->GetLookAtPt();

	//printf("EyePointO: %f %f %f\n", EyePointO.x, EyePointO.y, EyePointO.z);
	//printf("LookAtPointO: %f %f %f\n", LookAtPointO.x, LookAtPointO.y, LookAtPointO.z);

	XMVECTOR EyePoint;
	XMVECTOR LookAtPoint;

	EyePoint = XMLoadFloat3(&cam->GetEyePt());
	LookAtPoint = XMLoadFloat3(&cam->GetLookAtPt());

	EyePoint.m128_f32[1] = -1.0f*EyePoint.m128_f32[1] + 1.0f;
	LookAtPoint.m128_f32[1] = -1.0f*LookAtPoint.m128_f32[1] + 1.0f;

	XMMATRIX mView;
	XMMATRIX mProj;
	XMMATRIX mViewProj;
	XMMATRIX mViewProjInv;

	XMMATRIX mWorld;
	mWorld = XMLoadFloat4x4(&cam->GetWorldMatrix());

	XMFLOAT4X4 _mWorld;
	XMStoreFloat4x4(&_mWorld, mWorld);

	_mWorld._42 = -_mWorld._42 - 1.0f;
	_mWorld._21 *= -1.0f;
	_mWorld._23 *= -1.0f;
	_mWorld._32 *= -1.0f;

	mWorld = XMLoadFloat4x4(&_mWorld);

	mView = XMMatrixInverse(NULL, mWorld);

	mProj = XMLoadFloat4x4(&cam->GetProjMatrix());//  XMMatrixPerspectiveFovLH(camera_fov * XM_PI / 360.0f, aspectRatio, scene_z_near, scene_z_far);

	mViewProj = mView*mProj;
	XMStoreFloat4x4(&cbuffer.g_ModelViewProjectionMatrix, XMMatrixTranspose(mViewProj));

	XMStoreFloat3(&cbuffer.g_CameraPosition, EyePoint);

	XMVECTOR direction = LookAtPoint - EyePoint;
	XMVECTOR normalized_direction = XMVector3Normalize(direction);
	XMStoreFloat3(&cbuffer.g_CameraDirection, normalized_direction);

	cbuffer.g_HalfSpaceCullSign = 1.0f;
	cbuffer.g_HalfSpaceCullPosition = -0.6;
}