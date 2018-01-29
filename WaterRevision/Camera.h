#pragma once
#include "SimpleMath.h"

using namespace DirectX;
using namespace SimpleMath;

class Camera
{
public:
	Camera() {}
	~Camera() {}
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
