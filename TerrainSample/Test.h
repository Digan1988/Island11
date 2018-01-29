#pragma once
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class FRHIResource
{

};

class FRHIBlendState : public FRHIResource {};

class FD3D11BlendState : public FRHIBlendState
{
public:

	ComPtr<ID3D11BlendState> Resource;
};

typedef ID3D11Device FD3D11Device;

void test()
{
	ComPtr<FD3D11Device> Direct3DDevice;

	FD3D11BlendState* BlendState = new FD3D11BlendState();

	D3D11_BLEND_DESC BlendDesc;

	Direct3DDevice->CreateBlendState(&BlendDesc, BlendState->Resource.ReleaseAndGetAddressOf());
}