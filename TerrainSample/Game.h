//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include "CommonStates.h"
#include "Effects.h"
#include "PrimitiveBatch.h"
#include "GeometricPrimitive.h"
#include "SimpleMath.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Terrain.h"

using namespace DirectX;
using namespace SimpleMath;

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:

    Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();
    void Render();

    // Rendering helpers
    void Clear();
    void Present();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;


	void RenderTerrain();
	void InitTerrain();
	void updateCamera();
private:

    void Update(DX::StepTimer const& timer);

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

    // Device resources.
    HWND                                            m_window;
    int                                             m_outputWidth;
    int                                             m_outputHeight;

    D3D_FEATURE_LEVEL                               m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device>            m_d3dDevice;
    //Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice1;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>     m_d3dContext;
    //Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext1;

    Microsoft::WRL::ComPtr<IDXGISwapChain>          m_swapChain;
    Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain1;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;

	std::unique_ptr<DirectX::CommonStates> m_states;

	ID3D11SamplerState* SamplerAnisotropicWrap;
	ID3D11SamplerState* SamplerDepthAnisotropic;
	ID3D11SamplerState* SamplerLinearWrap;
	ID3D11SamplerState* SamplerLinearClamp;
	ID3D11SamplerState* SamplerPointClamp;

	ID3D11Buffer *m_pConstantBuffer;

	XMFLOAT4X4 m_world;
	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_proj;

	Vector3 cameraPos;
	Vector3 cameraLookAt;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_raster;

	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;

	float m_pitch;
	float m_yaw;
	float moveBackForward;
	float moveLeftRight;

	Terrain g_Terrain;
	Camera *camera;
};