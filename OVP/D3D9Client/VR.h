#pragma once

#include <vector>

#include <Windows.h>
#include <d3d11.h>
#include <d3d9.h>

#define XR_USE_GRAPHICS_API_D3D11
#define XR_USE_PLATFORM_WIN32
#include <openxr/openxr_platform.h>
#include <openxr/openxr.h>


struct DirectXSharedTexutre
{
	IDirect3DTexture9* d3d9 = nullptr;
	ID3D11Texture2D* d3d11 = nullptr;
	HANDLE shareHandle = nullptr;
};

class VR
{
public:
	VR(ID3D11Device* pDevice,
	   ID3D11DeviceContext* pContext,
	   IDirect3DDevice9Ex* pDevice9Ex
	);

	bool SubmitEyeTexture(int eye, LPDIRECT3DTEXTURE9 texture);

private:
	XrInstance instance;
	XrSession session;
	XrSwapchain eyeSwapchain[2];
	std::vector<XrSwapchainImageD3D11KHR> eyeSwapchainImages[2];

	ID3D11Device* pDevice;
	ID3D11DeviceContext* pContext;

	IDirect3DDevice9Ex* pDevice9Ex;
	DirectXSharedTexutre shareTextures[2];

	HRESULT CopyDX9Texture(LPDIRECT3DTEXTURE9 source, LPDIRECT3DTEXTURE9 dest);
};
