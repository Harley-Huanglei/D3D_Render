
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN // �� Windows ͷ���ų�����ʹ�õ�����
#include <windows.h>
#include <tchar.h>
//���WTL֧�� ����ʹ��COM
#include <wrl.h>
using namespace Microsoft;
using namespace Microsoft::WRL;

#include <dxgi1_6.h>
#include <DirectXMath.h>
using namespace DirectX;
//for d3d12
#include <d3d12.h>
#include <d3dcompiler.h>

//linker
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

//for WIC
#include <wincodec.h>

#include "d3dx12.h"

#define GRS_WND_CLASS_NAME _T("Game Window Class")
#define GRS_WND_TITLE	_T("DirectX12 Texture Sample")

#define GRS_THROW_IF_FAILED(hr) if (FAILED(hr)){ throw CGRSCOMException(hr); }

class CGRSCOMException
{
public:
	CGRSCOMException(HRESULT hr) : m_hrError(hr)
	{
	}
	HRESULT Error() const
	{
		return m_hrError;
	}
private:
	const HRESULT m_hrError;
};

struct WICTranslate
{
	GUID wic;
	DXGI_FORMAT format;
};

static WICTranslate g_WICFormats[] =
{//WIC��ʽ��DXGI���ظ�ʽ�Ķ�Ӧ���ñ��еĸ�ʽΪ��֧�ֵĸ�ʽ
	{ GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

	{ GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
	{ GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

	{ GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
	{ GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM }, // DXGI 1.1
	{ GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM }, // DXGI 1.1

	{ GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM }, // DXGI 1.1
	{ GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },

	{ GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
	{ GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

	{ GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
	{ GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
	{ GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
	{ GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

	{ GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },
};

// WIC ���ظ�ʽת����.
struct WICConvert
{
	GUID source;
	GUID target;
};

static WICConvert g_WICConvert[] =
{
	// Ŀ���ʽһ������ӽ��ı�֧�ֵĸ�ʽ
	{ GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

	{ GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

	{ GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM
	{ GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

	{ GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT
	{ GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT

	{ GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

	{ GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

	{ GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

	{ GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

	{ GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
	{ GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
	{ GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
	{ GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
	{ GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
	{ GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
	{ GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT

	{ GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
	{ GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
	{ GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
	{ GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
	{ GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT

	{ GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

	{ GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
	{ GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
	{ GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
};

bool GetTargetPixelFormat(const GUID* pSourceFormat, GUID* pTargetFormat)
{//���ȷ�����ݵ���ӽ���ʽ���ĸ�
	*pTargetFormat = *pSourceFormat;
	for (size_t i = 0; i < _countof(g_WICConvert); ++i)
	{
		if (InlineIsEqualGUID(g_WICConvert[i].source, *pSourceFormat))
		{
			*pTargetFormat = g_WICConvert[i].target;
			return true;
		}
	}
	return false;
}

DXGI_FORMAT GetDXGIFormatFromPixelFormat(const GUID* pPixelFormat)
{//���ȷ�����ն�Ӧ��DXGI��ʽ����һ��
	for (size_t i = 0; i < _countof(g_WICFormats); ++i)
	{
		if (InlineIsEqualGUID(g_WICFormats[i].wic, *pPixelFormat))
		{
			return g_WICFormats[i].format;
		}
	}
	return DXGI_FORMAT_UNKNOWN;
}

struct GRS_VERTEX
{
	XMFLOAT3 m_vPos;		//Position
	XMFLOAT2 m_vTxc;		//Texcoord
};

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR    lpCmdLine, int nCmdShow)
{
	::CoInitialize(nullptr);  //for WIC & COM

	const UINT nFrameBackBufCount = 3u;

	int iWidth = 1024;
	int iHeight = 768;
	UINT nFrameIndex = 0;
	UINT nFrame = 0;

	UINT nDXGIFactoryFlags = 0U;
	UINT nRTVDescriptorSize = 0U;

	HWND hWnd = nullptr;
	MSG	msg = {};

	float fAspectRatio = 3.0f;

	D3D12_VERTEX_BUFFER_VIEW stVertexBufferView = {};

	UINT64 n64FenceValue = 0ui64;
	HANDLE hFenceEvent = nullptr;

	UINT nTextureW = 0u;
	UINT nTextureH = 0u;
	UINT nBPP = 0u;
	DXGI_FORMAT stTextureFormat = DXGI_FORMAT_UNKNOWN;

	CD3DX12_VIEWPORT stViewPort(0.0f, 0.0f, static_cast<float>(iWidth), static_cast<float>(iHeight));
	CD3DX12_RECT	 stScissorRect(0, 0, static_cast<LONG>(iWidth), static_cast<LONG>(iHeight));

	ComPtr<IDXGIFactory5>				pIDXGIFactory5;
	ComPtr<IDXGIAdapter1>				pIAdapter;
	ComPtr<ID3D12Device4>				pID3DDevice;
	ComPtr<ID3D12CommandQueue>			pICommandQueue;
	ComPtr<IDXGISwapChain1>				pISwapChain1;
	ComPtr<IDXGISwapChain3>				pISwapChain3;
	ComPtr<ID3D12DescriptorHeap>		pIRTVHeap;
	ComPtr<ID3D12DescriptorHeap>		pISRVHeap;
	ComPtr<ID3D12Resource>				pIARenderTargets[nFrameBackBufCount];
	ComPtr<ID3D12Resource>				pITexcute;
	ComPtr<ID3D12CommandAllocator>		pICommandAllocator;
	ComPtr<ID3D12RootSignature>			pIRootSignature;
	ComPtr<ID3D12PipelineState>			pIPipelineState;
	ComPtr<ID3D12GraphicsCommandList>	pICommandList;
	ComPtr<ID3D12Resource>				pIVertexBuffer;
	ComPtr<ID3D12Fence>					pIFence;

	ComPtr<IWICImagingFactory>			pIWICFactory;
	ComPtr<IWICBitmapDecoder>			pIWICDecoder;
	ComPtr<IWICBitmapFrameDecode>		pIWICFrame;

	try
	{
		//---------------------------------------------------------------------------------------------
		// 1����������
		WNDCLASSEX wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_GLOBALCLASS;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);		//��ֹ���ĵı����ػ�
		wcex.lpszClassName = GRS_WND_CLASS_NAME;
		RegisterClassEx(&wcex);

		DWORD dwWndStyle = WS_OVERLAPPED | WS_SYSMENU;
		RECT rtWnd = { 0, 0, iWidth, iHeight };
		AdjustWindowRect(&rtWnd, dwWndStyle, FALSE);

		hWnd = CreateWindowW(GRS_WND_CLASS_NAME
			, GRS_WND_TITLE
			, dwWndStyle
			, CW_USEDEFAULT
			, 0
			, rtWnd.right - rtWnd.left
			, rtWnd.bottom - rtWnd.top
			, nullptr
			, nullptr
			, hInstance
			, nullptr);

		if (!hWnd)
		{
			return FALSE;
		}

		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		//---------------------------------------------------------------------------------------------
#if defined(_DEBUG)
		{//����ʾ��ϵͳ�ĵ���֧��
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
				// �򿪸��ӵĵ���֧��
				nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif
		//---------------------------------------------------------------------------------------------
		//2������DXGI Factory����
		GRS_THROW_IF_FAILED(CreateDXGIFactory2(nDXGIFactoryFlags, IID_PPV_ARGS(&pIDXGIFactory5)));
		// �ر�ALT+ENTER���л�ȫ���Ĺ��ܣ���Ϊ����û��ʵ��OnSize���������ȹر�
		GRS_THROW_IF_FAILED(pIDXGIFactory5->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

		//---------------------------------------------------------------------------------------------
		//3��ö������������ѡ����ʵ�������������3D�豸����
		for (UINT adapterIndex = 1; DXGI_ERROR_NOT_FOUND != pIDXGIFactory5->EnumAdapters1(adapterIndex, &pIAdapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc = {};
			pIAdapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{//������������������豸
				continue;
			}
			//�����������D3D֧�ֵļ��ݼ�������ֱ��Ҫ��֧��12.1��������ע�ⷵ�ؽӿڵ��Ǹ���������Ϊ��nullptr������
			//�Ͳ���ʵ�ʴ���һ���豸�ˣ�Ҳ�������ǆ��µ��ٵ���release���ͷŽӿڡ���Ҳ��һ����Ҫ�ļ��ɣ����ס��
			if (SUCCEEDED(D3D12CreateDevice(pIAdapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}

		//---------------------------------------------------------------------------------------------
		//4������D3D12.1���豸
		GRS_THROW_IF_FAILED(D3D12CreateDevice(pIAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pID3DDevice)));

		//---------------------------------------------------------------------------------------------
		//5������ֱ���������
		D3D12_COMMAND_QUEUE_DESC stQueueDesc = {};
		stQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		GRS_THROW_IF_FAILED(pID3DDevice->CreateCommandQueue(&stQueueDesc, IID_PPV_ARGS(&pICommandQueue)));

		//---------------------------------------------------------------------------------------------
		//6������������
		DXGI_SWAP_CHAIN_DESC1 stSwapChainDesc = {};
		stSwapChainDesc.BufferCount = nFrameBackBufCount;
		stSwapChainDesc.Width = iWidth;
		stSwapChainDesc.Height = iHeight;
		stSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		stSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		stSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		stSwapChainDesc.SampleDesc.Count = 1;

		GRS_THROW_IF_FAILED(pIDXGIFactory5->CreateSwapChainForHwnd(
			pICommandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
			hWnd,
			&stSwapChainDesc,
			nullptr,
			nullptr,
			&pISwapChain1
		));

		//---------------------------------------------------------------------------------------------
		//7���õ���ǰ�󻺳�������ţ�Ҳ������һ����Ҫ������ʾ�Ļ����������
		//ע��˴�ʹ���˸߰汾��SwapChain�ӿڵĺ���
		GRS_THROW_IF_FAILED(pISwapChain1.As(&pISwapChain3));
		nFrameIndex = pISwapChain3->GetCurrentBackBufferIndex();

		//---------------------------------------------------------------------------------------------
		//8������RTV(��ȾĿ����ͼ)��������(����ѵĺ���Ӧ�����Ϊ������߹̶���СԪ�صĹ̶���С�Դ��)
		D3D12_DESCRIPTOR_HEAP_DESC stRTVHeapDesc = {};
		stRTVHeapDesc.NumDescriptors = nFrameBackBufCount;
		stRTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		stRTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		GRS_THROW_IF_FAILED(pID3DDevice->CreateDescriptorHeap(&stRTVHeapDesc, IID_PPV_ARGS(&pIRTVHeap)));
		//�õ�ÿ��������Ԫ�صĴ�С
		nRTVDescriptorSize = pID3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		//---------------------------------------------------------------------------------------------
		//9������RTV��������
		CD3DX12_CPU_DESCRIPTOR_HANDLE stRTVHandle(pIRTVHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < nFrameBackBufCount; i++)
		{
			GRS_THROW_IF_FAILED(pISwapChain3->GetBuffer(i, IID_PPV_ARGS(&pIARenderTargets[i])));
			pID3DDevice->CreateRenderTargetView(pIARenderTargets[i].Get(), nullptr, stRTVHandle);
			stRTVHandle.Offset(1, nRTVDescriptorSize);
		}

		//---------------------------------------------------------------------------------------------
		//10������SRV�� (Shader Resource View Heap)
		D3D12_DESCRIPTOR_HEAP_DESC stSRVHeapDesc = {};
		stSRVHeapDesc.NumDescriptors = 1;
		stSRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		stSRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		GRS_THROW_IF_FAILED(pID3DDevice->CreateDescriptorHeap(&stSRVHeapDesc, IID_PPV_ARGS(&pISRVHeap)));

		//---------------------------------------------------------------------------------------------
		//11��������������
		D3D12_FEATURE_DATA_ROOT_SIGNATURE stFeatureData = {};
		// ����Ƿ�֧��V1.1�汾�ĸ�ǩ��
		stFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(pID3DDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &stFeatureData, sizeof(stFeatureData))))
		{
			stFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		// ��GPU��ִ��SetGraphicsRootDescriptorTable�����ǲ��޸������б��е�SRV��������ǿ���ʹ��Ĭ��Rang��Ϊ:
		// D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
		CD3DX12_DESCRIPTOR_RANGE1 stDSPRanges[1];
		stDSPRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE);

		CD3DX12_ROOT_PARAMETER1 stRootParameters[1];
		stRootParameters[0].InitAsDescriptorTable(1, &stDSPRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		D3D12_STATIC_SAMPLER_DESC stSamplerDesc = {};
		stSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		stSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		stSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		stSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		stSamplerDesc.MipLODBias = 0;
		stSamplerDesc.MaxAnisotropy = 0;
		stSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		stSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		stSamplerDesc.MinLOD = 0.0f;
		stSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		stSamplerDesc.ShaderRegister = 0;
		stSamplerDesc.RegisterSpace = 0;
		stSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC stRootSignatureDesc;
		stRootSignatureDesc.Init_1_1(_countof(stRootParameters), stRootParameters
			, 1, &stSamplerDesc
			, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> pISignatureBlob;
		ComPtr<ID3DBlob> pIErrorBlob;
		GRS_THROW_IF_FAILED(D3DX12SerializeVersionedRootSignature(&stRootSignatureDesc
			, stFeatureData.HighestVersion
			, &pISignatureBlob
			, &pIErrorBlob));
		GRS_THROW_IF_FAILED(pID3DDevice->CreateRootSignature(0
			, pISignatureBlob->GetBufferPointer()
			, pISignatureBlob->GetBufferSize()
			, IID_PPV_ARGS(&pIRootSignature)));

		//---------------------------------------------------------------------------------------------
		// 12������Shader������Ⱦ����״̬����
		ComPtr<ID3DBlob> pIBlobVertexShader;
		ComPtr<ID3DBlob> pIBlobPixelShader;
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		TCHAR pszShaderFileName[] = _T("Texture.hlsl");
		GRS_THROW_IF_FAILED(D3DCompileFromFile(pszShaderFileName, nullptr, nullptr
			, "VSMain", "vs_5_0", compileFlags, 0, &pIBlobVertexShader, nullptr));
		GRS_THROW_IF_FAILED(D3DCompileFromFile(pszShaderFileName, nullptr, nullptr
			, "PSMain", "ps_5_0", compileFlags, 0, &pIBlobPixelShader, nullptr));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC stInputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// ���� graphics pipeline state object (PSO)����
		D3D12_GRAPHICS_PIPELINE_STATE_DESC stPSODesc = {};
		stPSODesc.InputLayout = { stInputElementDescs, _countof(stInputElementDescs) };
		stPSODesc.pRootSignature = pIRootSignature.Get();
		stPSODesc.VS = CD3DX12_SHADER_BYTECODE(pIBlobVertexShader.Get());
		stPSODesc.PS = CD3DX12_SHADER_BYTECODE(pIBlobPixelShader.Get());
		stPSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		stPSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		stPSODesc.DepthStencilState.DepthEnable = FALSE;
		stPSODesc.DepthStencilState.StencilEnable = FALSE;
		stPSODesc.SampleMask = UINT_MAX;
		stPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		stPSODesc.NumRenderTargets = 1;
		stPSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		stPSODesc.SampleDesc.Count = 1;

		GRS_THROW_IF_FAILED(pID3DDevice->CreateGraphicsPipelineState(&stPSODesc
			, IID_PPV_ARGS(&pIPipelineState)));

		//---------------------------------------------------------------------------------------------
		// 13�����������б������
		GRS_THROW_IF_FAILED(pID3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT
			, IID_PPV_ARGS(&pICommandAllocator)));

		//---------------------------------------------------------------------------------------------
		// 14������ͼ�������б�
		GRS_THROW_IF_FAILED(pID3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT
			, pICommandAllocator.Get(), pIPipelineState.Get(), IID_PPV_ARGS(&pICommandList)));

		//---------------------------------------------------------------------------------------------
		// 15���������㻺��
		// ���������ε�3D���ݽṹ
		GRS_VERTEX stTriangleVertices[] =
		{
			{ { -0.25f * fAspectRatio, -0.25f * fAspectRatio, 0.0f}, { 0.0f, 1.0f } },	// Bottom left.
			{ { -0.25f * fAspectRatio, 0.25f * fAspectRatio, 0.0f}, { 0.0f, 0.0f } },	// Top left.
			{ { 0.25f * fAspectRatio, -0.25f * fAspectRatio, 0.0f }, { 1.0f, 1.0f } },	// Bottom right.
			{ { 0.25f * fAspectRatio, 0.25f * fAspectRatio, 0.0f}, { 1.0f, 0.0f } },		// Top right.
		};

		const UINT nVertexBufferSize = sizeof(stTriangleVertices);
		D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_RESOURCE_DESC  resDesc = CD3DX12_RESOURCE_DESC::Buffer(nVertexBufferSize);
		GRS_THROW_IF_FAILED(pID3DDevice->CreateCommittedResource(
			&heapProperty,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pIVertexBuffer)));

		UINT8* pVertexDataBegin = nullptr;
		CD3DX12_RANGE stReadRange(0, 0);		// We do not intend to read from this resource on the CPU.
		GRS_THROW_IF_FAILED(pIVertexBuffer->Map(0, &stReadRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, stTriangleVertices, sizeof(stTriangleVertices));
		pIVertexBuffer->Unmap(0, nullptr);

		stVertexBufferView.BufferLocation = pIVertexBuffer->GetGPUVirtualAddress();
		stVertexBufferView.StrideInBytes = sizeof(GRS_VERTEX);
		stVertexBufferView.SizeInBytes = nVertexBufferSize;

		//---------------------------------------------------------------------------------------------
		// 16��ʹ��WIC����������һ��2D����
		//ʹ�ô�COM��ʽ����WIC�೧����Ҳ�ǵ���WIC��һ��Ҫ��������
		GRS_THROW_IF_FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory)));

		//ʹ��WIC�೧����ӿڼ�������ͼƬ�����õ�һ��WIC����������ӿڣ�ͼƬ��Ϣ��������ӿڴ���Ķ�������
		WCHAR pszTexcuteFileName[] = _T("4k.jpg");
		GRS_THROW_IF_FAILED(pIWICFactory->CreateDecoderFromFilename(
			pszTexcuteFileName,              // �ļ���
			NULL,                            // ��ָ����������ʹ��Ĭ��
			GENERIC_READ,                    // ����Ȩ��
			WICDecodeMetadataCacheOnDemand,  // ����Ҫ�ͻ������� 
			&pIWICDecoder                    // ����������
		));

		// ��ȡ��һ֡ͼƬ(��ΪGIF�ȸ�ʽ�ļ����ܻ��ж�֡ͼƬ�������ĸ�ʽһ��ֻ��һ֡ͼƬ)
		// ʵ�ʽ���������������λͼ��ʽ����
		GRS_THROW_IF_FAILED(pIWICDecoder->GetFrame(0, &pIWICFrame));

		WICPixelFormatGUID wpf = {};
		//��ȡWICͼƬ��ʽ
		GRS_THROW_IF_FAILED(pIWICFrame->GetPixelFormat(&wpf));
		GUID tgFormat = {};

		//ͨ����һ��ת��֮���ȡDXGI�ĵȼ۸�ʽ
		if (GetTargetPixelFormat(&wpf, &tgFormat))
		{
			stTextureFormat = GetDXGIFormatFromPixelFormat(&tgFormat);
		}

		if (DXGI_FORMAT_UNKNOWN == stTextureFormat)
		{// ��֧�ֵ�ͼƬ��ʽ Ŀǰ�˳����� 
		 // һ�� ��ʵ�ʵ����浱�ж����ṩ�����ʽת�����ߣ�
		 // ͼƬ����Ҫ��ǰת���ã����Բ�����ֲ�֧�ֵ�����
			throw CGRSCOMException(S_FALSE);
		}

		// ����һ��λͼ��ʽ��ͼƬ���ݶ���ӿ�
		ComPtr<IWICBitmapSource>pIBMP;

		if (!InlineIsEqualGUID(wpf, tgFormat))
		{// ����жϺ���Ҫ�����ԭWIC��ʽ����ֱ����ת��ΪDXGI��ʽ��ͼƬʱ
		 // ������Ҫ���ľ���ת��ͼƬ��ʽΪ�ܹ�ֱ�Ӷ�ӦDXGI��ʽ����ʽ
			//����ͼƬ��ʽת����
			ComPtr<IWICFormatConverter> pIConverter;
			GRS_THROW_IF_FAILED(pIWICFactory->CreateFormatConverter(&pIConverter));

			//��ʼ��һ��ͼƬת������ʵ��Ҳ���ǽ�ͼƬ���ݽ����˸�ʽת��
			GRS_THROW_IF_FAILED(pIConverter->Initialize(
				pIWICFrame.Get(),                // ����ԭͼƬ����
				tgFormat,						 // ָ����ת����Ŀ���ʽ
				WICBitmapDitherTypeNone,         // ָ��λͼ�Ƿ��е�ɫ�壬�ִ��������λͼ�����õ�ɫ�壬����ΪNone
				NULL,                            // ָ����ɫ��ָ��
				0.f,                             // ָ��Alpha��ֵ
				WICBitmapPaletteTypeCustom       // ��ɫ�����ͣ�ʵ��û��ʹ�ã�����ָ��ΪCustom
			));
			// ����QueryInterface������ö����λͼ����Դ�ӿ�
			GRS_THROW_IF_FAILED(pIConverter.As(&pIBMP));
		}
		else
		{
			//ͼƬ���ݸ�ʽ����Ҫת����ֱ�ӻ�ȡ��λͼ����Դ�ӿ�
			GRS_THROW_IF_FAILED(pIWICFrame.As(&pIBMP));
		}
		//���ͼƬ��С����λ�����أ�
		GRS_THROW_IF_FAILED(pIBMP->GetSize(&nTextureW, &nTextureH));

		//��ȡͼƬ���ص�λ��С��BPP��Bits Per Pixel����Ϣ�����Լ���ͼƬ�����ݵ���ʵ��С����λ���ֽڣ�
		ComPtr<IWICComponentInfo> pIWICmntinfo;
		GRS_THROW_IF_FAILED(pIWICFactory->CreateComponentInfo(tgFormat, pIWICmntinfo.GetAddressOf()));

		WICComponentType type;
		GRS_THROW_IF_FAILED(pIWICmntinfo->GetComponentType(&type));

		if (type != WICPixelFormat)
		{
			throw CGRSCOMException(S_FALSE);
		}

		ComPtr<IWICPixelFormatInfo> pIWICPixelinfo;
		GRS_THROW_IF_FAILED(pIWICmntinfo.As(&pIWICPixelinfo));

		// ���������ڿ��Եõ�BPP�ˣ���Ҳ���ҿ��ıȽ���Ѫ�ĵط���Ϊ��BPP��Ȼ������ô�໷��
		GRS_THROW_IF_FAILED(pIWICPixelinfo->GetBitsPerPixel(&nBPP));

		// ����ͼƬʵ�ʵ��д�С����λ���ֽڣ�������ʹ����һ����ȡ����������A+B-1��/B ��
		// ����������˵��΢���������,ϣ�����Ѿ���������ָ��
		UINT nPicRowPitch = (uint64_t(nTextureW) * uint64_t(nBPP) + 7u) / 8u;
		//---------------------------------------------------------------------------------------------

		ComPtr<ID3D12Resource> pITextureUpload;

		D3D12_RESOURCE_DESC stTextureDesc = {};
		stTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		stTextureDesc.MipLevels = 1;
		stTextureDesc.Format = stTextureFormat; //DXGI_FORMAT_R8G8B8A8_UNORM;
		stTextureDesc.Width = nTextureW;
		stTextureDesc.Height = nTextureH;
		stTextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		stTextureDesc.DepthOrArraySize = 1;
		stTextureDesc.SampleDesc.Count = 1;
		stTextureDesc.SampleDesc.Quality = 0;


		//����Ĭ�϶��ϵ���Դ��������Texture2D��GPU��Ĭ�϶���Դ�ķ����ٶ�������
		//��Ϊ������Դһ���ǲ��ױ����Դ����������ͨ��ʹ���ϴ��Ѹ��Ƶ�Ĭ�϶���
		//�ڴ�ͳ��D3D11����ǰ��D3D�ӿ��У���Щ���̶�����װ�ˣ�����ֻ��ָ������ʱ������ΪĬ�϶� 
		heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		GRS_THROW_IF_FAILED(pID3DDevice->CreateCommittedResource(
			&heapProperty
			, D3D12_HEAP_FLAG_NONE
			, &stTextureDesc				//����ʹ��CD3DX12_RESOURCE_DESC::Tex2D���򻯽ṹ��ĳ�ʼ��
			, D3D12_RESOURCE_STATE_COPY_DEST
			, nullptr
			, IID_PPV_ARGS(&pITexcute)));

		//��ȡ�ϴ�����Դ����Ĵ�С������ߴ�ͨ������ʵ��ͼƬ�ĳߴ�
		const UINT64 n64UploadBufferSize = GetRequiredIntermediateSize(pITexcute.Get(), 0, 1);

		// ���������ϴ��������Դ,ע����������Buffer
		// �ϴ��Ѷ���GPU������˵�����Ǻܲ�ģ�
		// ���Զ��ڼ������������������������
		// ͨ�������ϴ���GPU���ʸ���Ч��Ĭ�϶���
		heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		resDesc = CD3DX12_RESOURCE_DESC::Buffer(n64UploadBufferSize);
		GRS_THROW_IF_FAILED(pID3DDevice->CreateCommittedResource(
			&heapProperty,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pITextureUpload)));

		//������Դ�����С������ʵ��ͼƬ���ݴ洢���ڴ��С
		void* pbPicData = ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, n64UploadBufferSize);
		if (nullptr == pbPicData)
		{
			throw CGRSCOMException(HRESULT_FROM_WIN32(GetLastError()));
		}

		//��ͼƬ�ж�ȡ������
		GRS_THROW_IF_FAILED(pIBMP->CopyPixels(nullptr
			, nPicRowPitch
			, static_cast<UINT>(nPicRowPitch * nTextureH)   //ע���������ͼƬ������ʵ�Ĵ�С�����ֵͨ��С�ڻ���Ĵ�С
			, reinterpret_cast<BYTE*>(pbPicData)));

		//{//������δ�������DX12��ʾ����ֱ��ͨ����仺�������һ���ڰ׷��������
		// //��ԭ��δ��룬Ȼ��ע�������CopyPixels���ÿ��Կ����ڰ׷��������Ч��
		//	const UINT rowPitch = nPicRowPitch; //nTextureW * 4; //static_cast<UINT>(n64UploadBufferSize / nTextureH);
		//	const UINT cellPitch = rowPitch >> 3;		// The width of a cell in the checkboard texture.
		//	const UINT cellHeight = nTextureW >> 3;	// The height of a cell in the checkerboard texture.
		//	const UINT textureSize = static_cast<UINT>(n64UploadBufferSize);
		//	UINT nTexturePixelSize = static_cast<UINT>(n64UploadBufferSize / nTextureH / nTextureW);

		//	UINT8* pData = reinterpret_cast<UINT8*>(pbPicData);

		//	for (UINT n = 0; n < textureSize; n += nTexturePixelSize)
		//	{
		//		UINT x = n % rowPitch;
		//		UINT y = n / rowPitch;
		//		UINT i = x / cellPitch;
		//		UINT j = y / cellHeight;

		//		if (i % 2 == j % 2)
		//		{
		//			pData[n] = 0x00;		// R
		//			pData[n + 1] = 0x00;	// G
		//			pData[n + 2] = 0x00;	// B
		//			pData[n + 3] = 0xff;	// A
		//		}
		//		else
		//		{
		//			pData[n] = 0xff;		// R
		//			pData[n + 1] = 0xff;	// G
		//			pData[n + 2] = 0xff;	// B
		//			pData[n + 3] = 0xff;	// A
		//		}
		//	}
		//}

		//��ȡ���ϴ��ѿ����������ݵ�һЩ����ת���ߴ���Ϣ
		//���ڸ��ӵ�DDS�������Ƿǳ���Ҫ�Ĺ���
		UINT64 n64RequiredSize = 0u;
		UINT   nNumSubresources = 1u;  //����ֻ��һ��ͼƬ��������Դ����Ϊ1
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT stTxtLayouts = {};
		UINT64 n64TextureRowSizes = 0u;
		UINT   nTextureRowNum = 0u;

		D3D12_RESOURCE_DESC stDestDesc = pITexcute->GetDesc();

		pID3DDevice->GetCopyableFootprints(&stDestDesc
			, 0
			, nNumSubresources
			, 0
			, &stTxtLayouts
			, &nTextureRowNum
			, &n64TextureRowSizes
			, &n64RequiredSize);

		//��Ϊ�ϴ���ʵ�ʾ���CPU�������ݵ�GPU���н�
		//�������ǿ���ʹ����Ϥ��Map����������ӳ�䵽CPU�ڴ��ַ��
		//Ȼ�����ǰ��н����ݸ��Ƶ��ϴ�����
		//��Ҫע�����֮���԰��п�������ΪGPU��Դ���д�С
		//��ʵ��ͼƬ���д�С���в����,���ߵ��ڴ�߽����Ҫ���ǲ�һ����
		BYTE* pData = nullptr;
		GRS_THROW_IF_FAILED(pITextureUpload->Map(0, NULL, reinterpret_cast<void**>(&pData)));

		BYTE* pDestSlice = reinterpret_cast<BYTE*>(pData) + stTxtLayouts.Offset;
		const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(pbPicData);
		for (UINT y = 0; y < nTextureRowNum; ++y)
		{
			memcpy(pDestSlice + static_cast<SIZE_T>(stTxtLayouts.Footprint.RowPitch) * y
				, pSrcSlice + static_cast<SIZE_T>(nPicRowPitch) * y
				, nPicRowPitch);
		}
		//ȡ��ӳ�� �����ױ��������ÿ֡�ı任��������ݣ�������������Unmap�ˣ�
		//������פ�ڴ�,������������ܣ���Ϊÿ��Map��Unmap�Ǻܺ�ʱ�Ĳ���
		//��Ϊ�������붼��64λϵͳ��Ӧ���ˣ���ַ�ռ����㹻�ģ�������ռ�ò���Ӱ��ʲô
		pITextureUpload->Unmap(0, NULL);

		//�ͷ�ͼƬ���ݣ���һ���ɾ��ĳ���Ա
		::HeapFree(::GetProcessHeap(), 0, pbPicData);

		//��������з������ϴ��Ѹ����������ݵ�Ĭ�϶ѵ�����
		CD3DX12_TEXTURE_COPY_LOCATION Dst(pITexcute.Get(), 0);
		CD3DX12_TEXTURE_COPY_LOCATION Src(pITextureUpload.Get(), stTxtLayouts);
		pICommandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);

		//����һ����Դ���ϣ�ͬ����ȷ�ϸ��Ʋ������
		//ֱ��ʹ�ýṹ��Ȼ����õ���ʽ
		D3D12_RESOURCE_BARRIER stResBar = {};
		stResBar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		stResBar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		stResBar.Transition.pResource = pITexcute.Get();
		stResBar.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		stResBar.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		stResBar.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		pICommandList->ResourceBarrier(1, &stResBar);

		//����ʹ��D3DX12���еĹ�������õĵȼ���ʽ������ķ�ʽ�����һЩ
		//pICommandList->ResourceBarrier(1
		//	, &CD3DX12_RESOURCE_BARRIER::Transition(pITexcute.Get()
		//	, D3D12_RESOURCE_STATE_COPY_DEST
		//	, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		//);

		//---------------------------------------------------------------------------------------------
		// ���մ���SRV������
		D3D12_SHADER_RESOURCE_VIEW_DESC stSRVDesc = {};
		stSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		stSRVDesc.Format = stTextureDesc.Format;
		stSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		stSRVDesc.Texture2D.MipLevels = 1;
		pID3DDevice->CreateShaderResourceView(pITexcute.Get(), &stSRVDesc, pISRVHeap->GetCPUDescriptorHandleForHeapStart());

		//---------------------------------------------------------------------------------------------
		// ִ�������б��ȴ�������Դ�ϴ���ɣ���һ���Ǳ����
		GRS_THROW_IF_FAILED(pICommandList->Close());
		ID3D12CommandList* ppCommandLists[] = { pICommandList.Get() };
		pICommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//---------------------------------------------------------------------------------------------
		// 17������һ��ͬ�����󡪡�Χ�������ڵȴ���Ⱦ��ɣ���Ϊ����Draw Call���첽����
		GRS_THROW_IF_FAILED(pID3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pIFence)));
		n64FenceValue = 1;

		//---------------------------------------------------------------------------------------------
		// 18������һ��Eventͬ���������ڵȴ�Χ���¼�֪ͨ
		hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (hFenceEvent == nullptr)
		{
			GRS_THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
		}

		//---------------------------------------------------------------------------------------------
		// 19���ȴ�������Դ��ʽ���������
		const UINT64 fence = n64FenceValue;
		GRS_THROW_IF_FAILED(pICommandQueue->Signal(pIFence.Get(), fence));
		n64FenceValue++;

		//---------------------------------------------------------------------------------------------
		// ��������û������ִ�е�Χ����ǵ����û�о������¼�ȥ�ȴ���ע��ʹ�õ���������ж����ָ��
		if (pIFence->GetCompletedValue() < fence)
		{
			GRS_THROW_IF_FAILED(pIFence->SetEventOnCompletion(fence, hFenceEvent));
			WaitForSingleObject(hFenceEvent, INFINITE);
		}

		//---------------------------------------------------------------------------------------------
		//�����������Resetһ�£��ղ��Ѿ�ִ�й���һ���������������
		GRS_THROW_IF_FAILED(pICommandAllocator->Reset());
		//Reset�����б�������ָ�������������PSO����
		GRS_THROW_IF_FAILED(pICommandList->Reset(pICommandAllocator.Get(), pIPipelineState.Get()));
		//---------------------------------------------------------------------------------------------


		//---------------------------------------------------------------------------------------------
		//20��������ʱ�������Ա��ڴ�����Ч����Ϣѭ��
		HANDLE phWait = CreateWaitableTimer(NULL, FALSE, NULL);
		LARGE_INTEGER liDueTime = {};
		liDueTime.QuadPart = -1i64;//1���ʼ��ʱ
		SetWaitableTimer(phWait, &liDueTime, 1, NULL, NULL, 0);//40ms������

		//---------------------------------------------------------------------------------------------
		//21����ʼ��Ϣѭ�����������в�����Ⱦ
		DWORD dwRet = 0;
		BOOL bExit = FALSE;
		while (!bExit)
		{
			dwRet = ::MsgWaitForMultipleObjects(1, &phWait, FALSE, INFINITE, QS_ALLINPUT);
			switch (dwRet - WAIT_OBJECT_0)
			{
			case 0:
			case WAIT_TIMEOUT:
			{//��ʱ��ʱ�䵽
				//GRS_TRACE(_T("��ʼ��%u֡��Ⱦ{Frame Index = %u}��\n"),nFrame,nFrameIndex);
				//��ʼ��¼����
				//---------------------------------------------------------------------------------------------
				pICommandList->SetGraphicsRootSignature(pIRootSignature.Get());
				ID3D12DescriptorHeap* ppHeaps[] = { pISRVHeap.Get() };
				pICommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
				pICommandList->SetGraphicsRootDescriptorTable(0, pISRVHeap->GetGPUDescriptorHandleForHeapStart());
				pICommandList->RSSetViewports(1, &stViewPort);
				pICommandList->RSSetScissorRects(1, &stScissorRect);

				//---------------------------------------------------------------------------------------------
				// ͨ����Դ�����ж��󻺳��Ѿ��л���Ͽ��Կ�ʼ��Ⱦ��
				D3D12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pIARenderTargets[nFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
				pICommandList->ResourceBarrier(1, &resBarrier);

				CD3DX12_CPU_DESCRIPTOR_HANDLE stRTVHandle(pIRTVHeap->GetCPUDescriptorHandleForHeapStart(), nFrameIndex, nRTVDescriptorSize);
				//������ȾĿ��
				pICommandList->OMSetRenderTargets(1, &stRTVHandle, FALSE, nullptr);

				// ������¼�����������ʼ��һ֡����Ⱦ
				const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
				pICommandList->ClearRenderTargetView(stRTVHandle, clearColor, 0, nullptr);
				pICommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				pICommandList->IASetVertexBuffers(0, 1, &stVertexBufferView);

				//---------------------------------------------------------------------------------------------
				//Draw Call������
				pICommandList->DrawInstanced(_countof(stTriangleVertices), 1, 0, 0);

				//---------------------------------------------------------------------------------------------
				//��һ����Դ���ϣ�����ȷ����Ⱦ�Ѿ����������ύ����ȥ��ʾ��
				resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pIARenderTargets[nFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
				pICommandList->ResourceBarrier(1, &resBarrier);
				//�ر������б�����ȥִ����
				GRS_THROW_IF_FAILED(pICommandList->Close());

				//---------------------------------------------------------------------------------------------
				//ִ�������б�
				ID3D12CommandList* ppCommandLists[] = { pICommandList.Get() };
				pICommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

				//---------------------------------------------------------------------------------------------
				//�ύ����
				GRS_THROW_IF_FAILED(pISwapChain3->Present(1, 0));

				//---------------------------------------------------------------------------------------------
				//��ʼͬ��GPU��CPU��ִ�У��ȼ�¼Χ�����ֵ
				const UINT64 fence = n64FenceValue;
				GRS_THROW_IF_FAILED(pICommandQueue->Signal(pIFence.Get(), fence));
				n64FenceValue++;

				//---------------------------------------------------------------------------------------------
				// ��������û������ִ�е�Χ����ǵ����û�о������¼�ȥ�ȴ���ע��ʹ�õ���������ж����ָ��
				if (pIFence->GetCompletedValue() < fence)
				{
					GRS_THROW_IF_FAILED(pIFence->SetEventOnCompletion(fence, hFenceEvent));
					WaitForSingleObject(hFenceEvent, INFINITE);
				}
				//ִ�е�����˵��һ���������������ִ�����ˣ�������ʹ������ǵ�һ֡�Ѿ���Ⱦ���ˣ�����׼��ִ����һ֡��Ⱦ

				//---------------------------------------------------------------------------------------------
				//��ȡ�µĺ󻺳���ţ���ΪPresent�������ʱ�󻺳����ž͸�����
				nFrameIndex = pISwapChain3->GetCurrentBackBufferIndex();

				//---------------------------------------------------------------------------------------------
				//�����������Resetһ��
				GRS_THROW_IF_FAILED(pICommandAllocator->Reset());
				//Reset�����б�������ָ�������������PSO����
				GRS_THROW_IF_FAILED(pICommandList->Reset(pICommandAllocator.Get(), pIPipelineState.Get()));

				//GRS_TRACE(_T("��%u֡��Ⱦ����.\n"), nFrame++);
			}
			break;
			case 1:
			{//������Ϣ
				while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (WM_QUIT != msg.message)
					{
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
					else
					{
						bExit = TRUE;
					}
				}
			}
			break;
			default:
				break;
			}
		}
		//::CoUninitialize();
	}
	catch (CGRSCOMException& e)
	{//������COM�쳣
		e;
	}


	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
