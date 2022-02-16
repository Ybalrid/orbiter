#include "VR.h"
#include <vector>

#include "xrew.h"

VR::VR(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, IDirect3DDevice9Ex* pDevice9Ex): pDevice(pDevice), pContext(pContext), pDevice9Ex(pDevice9Ex)
{
	std::vector<const char *> requiredExtensions{ XR_KHR_D3D11_ENABLE_EXTENSION_NAME };

	uint32_t extensionPropertiesCount = 0;
	xrEnumerateInstanceExtensionProperties(nullptr, extensionPropertiesCount, &extensionPropertiesCount, nullptr);
	std::vector<XrExtensionProperties> availableExtensions(extensionPropertiesCount, { XR_TYPE_EXTENSION_PROPERTIES });
	xrEnumerateInstanceExtensionProperties(nullptr, availableExtensions.size(), &extensionPropertiesCount, availableExtensions.data());

	bool hasAllExtensions = true;
	for(const auto requiredExtensionName  : requiredExtensions)
	{
		const auto foundExtension = std::find_if(availableExtensions.cbegin(), availableExtensions.cend(), [requiredExtensionName](const XrExtensionProperties& availableExtensionProperties)
			{
				return 0 == std::strcmp(requiredExtensionName, availableExtensionProperties.extensionName);
			});

		if(foundExtension == availableExtensions.cend())
		{
			hasAllExtensions = false;
			break;
		}
	}

	if(!hasAllExtensions)
	{
		//todo report errors
	}

	std::vector<const char*> enabledExtensions{};
	enabledExtensions.insert(enabledExtensions.end(), requiredExtensions.cbegin(), requiredExtensions.cend());

	XrInstanceCreateInfo instanceCreateInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
	std::strcpy(instanceCreateInfo.applicationInfo.applicationName, "Orbiter2016");
	instanceCreateInfo.applicationInfo.applicationVersion = XR_MAKE_VERSION(0, 0, 0);
	instanceCreateInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

	instanceCreateInfo.enabledExtensionCount = enabledExtensions.size();
	instanceCreateInfo.enabledExtensionNames = enabledExtensions.data();

	if(XR_FAILED(xrCreateInstance(&instanceCreateInfo, &instance)))
	{
		//todo report errors
	}

	xrewInit(instance);

	XrSystemId systemId;
	XrSystemGetInfo systemGetInfo{ XR_TYPE_SYSTEM_GET_INFO, nullptr, XrFormFactor::XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY };
	xrGetSystem(instance, &systemGetInfo, &systemId);

	XrSystemProperties systemProperties{ XR_TYPE_SYSTEM_PROPERTIES, nullptr, systemId };
	xrGetSystemProperties(instance, systemId, &systemProperties);

	XrGraphicsRequirementsD3D11KHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
	xrGetD3D11GraphicsRequirementsKHR(instance, systemId, &graphicsRequirements);

	XrGraphicsBindingD3D11KHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_D3D11_KHR, nullptr, pDevice };
	XrSessionCreateInfo sessionCreateInfo{ XR_TYPE_SESSION_CREATE_INFO, &graphicsBinding,0, systemId };
	xrCreateSession(instance, &sessionCreateInfo, &session);

	uint32_t nbFormats = 0;
	xrEnumerateSwapchainFormats(session, 0, &nbFormats, nullptr);
	std::vector<int64_t> swapchainFormats(nbFormats);
	xrEnumerateSwapchainFormats(session, swapchainFormats.size(), &nbFormats, swapchainFormats.data());
	std::vector<DXGI_FORMAT> swapchainDXGIFormats(nbFormats);
	for (size_t i = 0; i < nbFormats; ++i)
		swapchainDXGIFormats[i] = (DXGI_FORMAT)swapchainFormats[i];

	const auto hasR8G8B8A8 = std::find(swapchainDXGIFormats.begin(), swapchainDXGIFormats.end(), DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) != swapchainDXGIFormats.end();
	if(!hasR8G8B8A8)
	{
		//well that is embarrassing, innit?
	}

	XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO, nullptr };
	swapchainCreateInfo.arraySize = 1;
	swapchainCreateInfo.faceCount = 1;
	swapchainCreateInfo.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	swapchainCreateInfo.mipCount = 1;
	swapchainCreateInfo.sampleCount = 1;
	swapchainCreateInfo.width = systemProperties.graphicsProperties.maxSwapchainImageWidth;
	swapchainCreateInfo.height = systemProperties.graphicsProperties.maxSwapchainImageHeight;
	swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT; // only thing we will do with this is to copy pixels into it

	for (int i = 0; i < 2; ++i)
	{
		xrCreateSwapchain(session, &swapchainCreateInfo, &eyeSwapchain[i]);
		pDevice9Ex->CreateTexture(swapchainCreateInfo.width, swapchainCreateInfo.height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &shareTextures[i].d3d9, &shareTextures[i].shareHandle);
		pDevice->OpenSharedResource(shareTextures[i].shareHandle, __uuidof(ID3D11Texture2D), (void**)&shareTextures[i].d3d11);

		uint32_t nbImages = 0;
		xrEnumerateSwapchainImages(eyeSwapchain[i], 0, &nbImages, nullptr);
		std::vector < XrSwapchainImageD3D11KHR > swapchainImages(nbImages,  {XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
		xrEnumerateSwapchainImages(eyeSwapchain[i], swapchainImages.size(), &nbImages, (XrSwapchainImageBaseHeader*)swapchainImages.data());
		eyeSwapchainImages[i] = swapchainImages;
	}
}

bool VR::SubmitEyeTexture(int eye, LPDIRECT3DTEXTURE9 texture)
{
	//step 1, copy the texture to the shared texure
	CopyDX9Texture(texture, shareTextures[eye].d3d9);

	XrSwapchainImageAcquireInfo swapchainImageAcquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	uint32_t swapchainIndex = -1;

	xrAcquireSwapchainImage(eyeSwapchain[eye], &swapchainImageAcquireInfo, &swapchainIndex);

	XrSwapchainImageWaitInfo swapchainImageWaitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	swapchainImageWaitInfo.timeout = 1000;
	xrWaitSwapchainImage(eyeSwapchain[eye], &swapchainImageWaitInfo);

	pContext->CopyResource(eyeSwapchainImages[eye][swapchainIndex].texture, shareTextures[eye].d3d11);

	XrSwapchainImageReleaseInfo swapchainImageReleaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	xrReleaseSwapchainImage(eyeSwapchain[eye], &swapchainImageReleaseInfo);

	return true;
}

HRESULT VR::CopyDX9Texture(LPDIRECT3DTEXTURE9 source, LPDIRECT3DTEXTURE9 dest)
{
	LPDIRECT3DSURFACE9 source_surface = nullptr, dest_surface = nullptr;
	const auto source_surface_ok = source->GetSurfaceLevel(0, &source_surface);
	const auto dest_surface_ok = dest->GetSurfaceLevel(0, &dest_surface);

	const auto result = (source_surface_ok == D3D_OK && dest_surface_ok == D3D_OK)
		? pDevice9Ex->StretchRect(source_surface,
			nullptr,
			dest_surface,
			nullptr,
			D3DTEXTUREFILTERTYPE::D3DTEXF_NONE)
		: D3DERR_INVALIDCALL;

	if (source_surface_ok == D3D_OK)
		source_surface->Release();
	if (dest_surface_ok == D3D_OK)
		dest_surface->Release();
	return result;
}

