#pragma once

#include "StepTimer.h"
#include "CommonStates.h"
#include "Effects.h"
#include "PrimitiveBatch.h"
#include "GeometricPrimitive.h"
#include "SimpleMath.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Camera.h"
#include "Terrain.h"

using namespace DirectX;
using namespace SimpleMath;

class Game
{
public:
	Game();
	~Game();
	void GetDefaultSize(int& width, int& height) const;
	void Initialize(HWND window, int width, int height);
	void Tick();
	void Render();

	void OnSuspending();
	void OnResuming();
	void OnWindowSizeChanged(int width, int height);
	void OnActivated();
	void OnDeactivated();
	void InitTerrain();
	void updateCamera();
	void Clear();
	void Present();
	void RenderTerrain();
private:
	void CreateDevice();
	void CreateResources();
	void Update(DX::StepTimer const& timer);
	void OnDeviceLost();

	HWND	m_window;
	int     m_outputWidth;
	int		m_outputHeight;
	D3D_FEATURE_LEVEL	m_featureLevel;
	Microsoft::WRL::ComPtr<ID3D11Device>            m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>     m_d3dContext;

	Microsoft::WRL::ComPtr<IDXGISwapChain>          m_swapChain;
	Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain1;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

	DX::StepTimer m_timer;
	std::unique_ptr<DirectX::CommonStates> m_states;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_raster;

	ID3D11SamplerState* SamplerAnisotropicWrap;
	ID3D11SamplerState* SamplerDepthAnisotropic;
	ID3D11SamplerState* SamplerLinearWrap;
	ID3D11SamplerState* SamplerLinearClamp;
	ID3D11SamplerState* SamplerPointClamp;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;

	ID3D11Buffer *m_pConstantBuffer;

	Camera *camera;
	Vector3 cameraPos;
	Vector3 cameraLookAt;
	float m_pitch;
	float m_yaw;
	float moveBackForward;
	float moveLeftRight;

	Terrain g_Terrain;
};
