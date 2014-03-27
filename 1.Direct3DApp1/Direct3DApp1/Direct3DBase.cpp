#include "pch.h"
#include "Direct3DBase.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

// 构造函数。
Direct3DBase::Direct3DBase()
{
}

// 初始化运行所需的 Direct3D 资源。
void Direct3DBase::Initialize(CoreWindow^ window)
{
	m_window = window;
	
	CreateDeviceResources();
	CreateWindowSizeDependentResources();
}

// 重新创建所有设备资源并将其设置回当前状态。
void Direct3DBase::HandleDeviceLost()
{
	// 重置这些成员变量，以确保 UpdateForWindowSizeChange 重新创建所有资源。
	m_windowBounds.Width = 0;
	m_windowBounds.Height = 0;
	m_swapChain = nullptr;

	CreateDeviceResources();
	UpdateForWindowSizeChange();
}

// 以下是依赖设备的资源。
void Direct3DBase::CreateDeviceResources()
{
	// 此标志为与 API 默认设置具有不同颜色渠道顺序的图面
	// 添加支持。要与 Direct2D 兼容，必须满足此要求。
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	// 如果项目处于调试生成过程中，请通过带有此标志的 SDK 层启用调试。
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// 此数组定义此应用程序将支持的 DirectX 硬件功能级别组。
	// 请注意，应保留顺序。
	// 请不要忘记在应用程序的说明中声明其所需的
	// 最低功能级别。除非另行说明，否则假定所有应用程序均支持 9.1。
	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// 创建 Direct3D 11 API 设备对象和对应的上下文。
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	DX::ThrowIfFailed(
		D3D11CreateDevice(
			nullptr, // 指定 nullptr 以使用默认适配器。
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			creationFlags, // 设置调试和 Direct2D 兼容性标志。
			featureLevels, // 此应用程序可以支持的功能级别的列表。
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION, // 对于 Windows 应用商店应用程序，始终将此值设置为 D3D11_SDK_VERSION。
			&device, // 返回创建的 Direct3D 设备。
			&m_featureLevel, // 返回所创建设备的功能级别。
			&context // 返回设备的即时上下文。
			)
		);

	// 获取 Direct3D 11.1 API 设备和上下文接口。
	DX::ThrowIfFailed(
		device.As(&m_d3dDevice)
		);

	DX::ThrowIfFailed(
		context.As(&m_d3dContext)
		);
}

// 分配在窗口 SizeChanged 事件中更改的所有内存资源。
void Direct3DBase::CreateWindowSizeDependentResources()
{ 
	// 存储窗口绑定，以便下次我们获取 SizeChanged 事件时，可避免
	// 在大小相同的情况下重新生成所有内容。
	m_windowBounds = m_window->Bounds;

	// 计算必要的交换链并呈现目标大小(以像素为单位)。
	float windowWidth = ConvertDipsToPixels(m_windowBounds.Width);
	float windowHeight = ConvertDipsToPixels(m_windowBounds.Height);

	// 交换链的宽度和高度必须基于窗口的
	// 横向宽度和高度。如果窗口在纵向
	// 方向，则必须使尺寸反转。
	m_orientation = DisplayProperties::CurrentOrientation;
	bool swapDimensions =
		m_orientation == DisplayOrientations::Portrait ||
		m_orientation == DisplayOrientations::PortraitFlipped;
	m_renderTargetSize.Width = swapDimensions ? windowHeight : windowWidth;
	m_renderTargetSize.Height = swapDimensions ? windowWidth : windowHeight;

	if(m_swapChain != nullptr)
	{
		// 如果交换链已存在，请调整其大小。
		DX::ThrowIfFailed(
			m_swapChain->ResizeBuffers(
				2, // 双缓冲交换链。
				static_cast<UINT>(m_renderTargetSize.Width),
				static_cast<UINT>(m_renderTargetSize.Height),
				DXGI_FORMAT_B8G8R8A8_UNORM,
				0
				)
			);
	}
	else
	{
		// 否则，使用与现有 Direct3D 设备相同的适配器新建一个。
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Width = static_cast<UINT>(m_renderTargetSize.Width); // 匹配窗口的大小。
		swapChainDesc.Height = static_cast<UINT>(m_renderTargetSize.Height);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 这是最常用的交换链格式。
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1; // 请不要使用多采样。
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2; // 使用双缓冲最大程度地减小延迟。
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // 所有 Windows 应用商店应用程序都必须使用此 SwapEffect。
		swapChainDesc.Flags = 0;

		ComPtr<IDXGIDevice1>  dxgiDevice;
		DX::ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice)
			);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		DX::ThrowIfFailed(
			dxgiDevice->GetAdapter(&dxgiAdapter)
			);

		ComPtr<IDXGIFactory2> dxgiFactory;
		DX::ThrowIfFailed(
			dxgiAdapter->GetParent(
				__uuidof(IDXGIFactory2), 
				&dxgiFactory
				)
			);

		Windows::UI::Core::CoreWindow^ window = m_window.Get();
		DX::ThrowIfFailed(
			dxgiFactory->CreateSwapChainForCoreWindow(
				m_d3dDevice.Get(),
				reinterpret_cast<IUnknown*>(window),
				&swapChainDesc,
				nullptr, // 在所有显示中都允许。
				&m_swapChain
				)
			);
			
		// 确保 DXGI 不会一次对多个帧排队。这样既可以减小延迟，
		// 又可以确保应用程序将只在每个 VSync 之后呈现，从而最大程度地降低功率消耗。
		DX::ThrowIfFailed(
			dxgiDevice->SetMaximumFrameLatency(1)
			);
	}
	
	// 为交换链设置正确方向，并生成
	// 用于呈现到旋转交换链的 3D 矩阵变换。
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;
	switch (m_orientation)
	{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			m_orientationTransform3D = XMFLOAT4X4( // 0 度 Z 旋转
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				);
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			m_orientationTransform3D = XMFLOAT4X4( // 90 度 Z 旋转
				0.0f, 1.0f, 0.0f, 0.0f,
				-1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				);
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			m_orientationTransform3D = XMFLOAT4X4( // 180 度 Z 旋转
				-1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				);
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			m_orientationTransform3D = XMFLOAT4X4( // 270 度 Z 旋转
				0.0f, -1.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				);
			break;

		default:
			throw ref new Platform::FailureException();
	}

	DX::ThrowIfFailed(
		m_swapChain->SetRotation(rotation)
		);

	// 创建交换链后台缓冲区的呈现目标视图。
	ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(
		m_swapChain->GetBuffer(
			0,
			__uuidof(ID3D11Texture2D),
			&backBuffer
			)
		);

	DX::ThrowIfFailed(
		m_d3dDevice->CreateRenderTargetView(
			backBuffer.Get(),
			nullptr,
			&m_renderTargetView
			)
		);

	// 创建深度模具视图。
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT, 
		static_cast<UINT>(m_renderTargetSize.Width),
		static_cast<UINT>(m_renderTargetSize.Height),
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
		);

	ComPtr<ID3D11Texture2D> depthStencil;
	DX::ThrowIfFailed(
		m_d3dDevice->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			&depthStencil
			)
		);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(
		m_d3dDevice->CreateDepthStencilView(
			depthStencil.Get(),
			&depthStencilViewDesc,
			&m_depthStencilView
			)
		);

	// 设置用于确定整个窗口的呈现视区。
	CD3D11_VIEWPORT viewport(
		0.0f,
		0.0f,
		m_renderTargetSize.Width,
		m_renderTargetSize.Height
		);

	m_d3dContext->RSSetViewports(1, &viewport);
}

// 在 SizeChanged 事件的事件处理程序中调用此方法。
void Direct3DBase::UpdateForWindowSizeChange()
{
	if (m_window->Bounds.Width  != m_windowBounds.Width ||
		m_window->Bounds.Height != m_windowBounds.Height ||
		m_orientation != DisplayProperties::CurrentOrientation)
	{
		ID3D11RenderTargetView* nullViews[] = {nullptr};
		m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
		m_renderTargetView = nullptr;
		m_depthStencilView = nullptr;
		m_d3dContext->Flush();
		CreateWindowSizeDependentResources();
	}
}

// 用于将最终图像传递给显示器的方法。
void Direct3DBase::Present()
{
	// 应用程序可以选择指定“已更新”或“滚动”
	// 矩形，以改进特定方案的效率。
	DXGI_PRESENT_PARAMETERS parameters = {0};
	parameters.DirtyRectsCount = 0;
	parameters.pDirtyRects = nullptr;
	parameters.pScrollRect = nullptr;
	parameters.pScrollOffset = nullptr;
	
	// 第一个参数指示 DXGI 进行阻止直到 VSync，这使应用程序
	// 在下一个 VSync 前进入休眠。这将确保我们不会浪费任何周期呈现将
	// 从不会在屏幕上显示的帧。
	HRESULT hr = m_swapChain->Present1(1, 0, &parameters);

	// 放弃呈现目标的内容。
	// 这是仅在完全覆盖现有内容时
	// 有效的操作。如果使用已更新或滚动矩形，则应将本调用删除。
	m_d3dContext->DiscardView(m_renderTargetView.Get());

	// 放弃深度模具的内容。
	m_d3dContext->DiscardView(m_depthStencilView.Get());

	// 如果通过断开连接或进行驱动程序升级的方法删除了设备，则必须
	// 必须重新创建所有设备资源。
	if (hr == DXGI_ERROR_DEVICE_REMOVED)
	{
		HandleDeviceLost();
	}
	else
	{
		DX::ThrowIfFailed(hr);
	}
}

// 用于将与设备无关的像素(DIP)长度转换为物理像素长度的方法。
float Direct3DBase::ConvertDipsToPixels(float dips)
{
	static const float dipsPerInch = 96.0f;
	return floor(dips * DisplayProperties::LogicalDpi / dipsPerInch + 0.5f); // 舍入到最接近的整数。
}
