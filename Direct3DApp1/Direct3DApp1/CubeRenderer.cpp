#include "pch.h"
#include "CubeRenderer.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

CubeRenderer::CubeRenderer() :
	m_loadingComplete(false),
	m_indexCount(0)
{
}

//CreateDeviceResources()实现创建设备资源的功能。CubeRenderer类继承自Direct3DBase类，
//CreateDeviceResources方法首先调用基类的CreateDeviceResources方法，
//然后读入生成的顶点着色器和像素着色器文件。注意ReadDataAsync方法中的Async表明这是一个异步方法。   
void CubeRenderer::CreateDeviceResources()
{
	Direct3DBase::CreateDeviceResources(); //base method

	//载入顶点着色器和像素着色器task
	auto loadVSTask = DX::ReadDataAsync("SimpleVertexShader.cso"); //顶点着色器
	auto loadPSTask = DX::ReadDataAsync("SimplePixelShader.cso"); //像素着色器

	//创建顶点着色器task，在载入顶点着色器成功之后，创建
	auto createVSTask = loadVSTask.then([this](Platform::Array<byte>^ fileData) {
		DX::ThrowIfFailed(
			m_d3dDevice->CreateVertexShader(
 				fileData->Data,
				fileData->Length,
				nullptr,
				&m_vertexShader
				)
			);

		const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_d3dDevice->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				fileData->Data,
				fileData->Length,
				&m_inputLayout
				)
			);
	});

	//创建像素着色器task,在载入像素着色器成功之后，创建
	auto createPSTask = loadPSTask.then([this](Platform::Array<byte>^ fileData) {
		DX::ThrowIfFailed(
			m_d3dDevice->CreatePixelShader(
				fileData->Data,
				fileData->Length,
				nullptr,
				&m_pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
				)
			);
	});

	//利用上两步的像素着色器和顶点着色器Task，创建cubeTask
	auto createCubeTask = (createPSTask && createVSTask).then([this] () {
		//顶点位置和颜色矩阵
		VertexPositionColor cubeVertices[] = 
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
				)
			);

		//三角面片
		unsigned short cubeIndices[] = 
		{
			0,2,1, // -x
			1,2,3,

			4,5,6, // +x
			5,7,6,

			0,1,5, // -y
			0,5,4,

			2,6,7, // +y
			2,7,3,

			0,4,6, // -z
			0,6,2,

			1,3,7, // +z
			1,7,5,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
				)
			);
	});

	//执行cubeTask，完成之后将标志置为true
	createCubeTask.then([this] () {
		m_loadingComplete = true;
	});
}

//CreateWindowSizeDependentResources
//方法首先根据窗口状态初始化显示比例，并设置可视角度为70度
//（DirectX只支持弧度，度转为弧度可通过常量XM_PI实现）；0.01f和100.0f则用来设置可见距离
void CubeRenderer::CreateWindowSizeDependentResources()
{
	Direct3DBase::CreateWindowSizeDependentResources();

	float aspectRatio = m_windowBounds.Width / m_windowBounds.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// 请注意，此处后乘 m_orientationTransform3D 矩阵，
	// 以正确确定场景的方向，使之与显示方向匹配。
	// 对于交换链的目标位图进行的任何绘制调用
	// 交换链呈现目标。对于到其他目标的绘制调用，
	// 不应应用此转换。
	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(
			XMMatrixMultiply(
				XMMatrixPerspectiveFovRH(
					fovAngleY,
					aspectRatio,
					0.01f,
					100.0f
					),
				XMLoadFloat4x4(&m_orientationTransform3D)
				)
			)
		);
}

//负责更新不同时刻正方体的旋转角度
void CubeRenderer::Update(float timeTotal, float timeDelta)
{
	(void) timeDelta; // 未使用的参数。

	XMVECTOR eye = XMVectorSet(0.0f, 0.7f, 1.5f, 0.0f); //眼睛所在位置
	XMVECTOR at = XMVectorSet(0.0f, -0.1f, 0.0f, 0.0f); //眼睛观察方向
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); //上方向


	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(timeTotal * XM_PIDIV4)));
}

//Render方法实现最终图像的绘制，其主要就是设置各个功能的缓冲区地址。
void CubeRenderer::Render()
{
	const float midnightBlue[] = { 0.098f, 0.098f, 0.439f, 1.000f };
	m_d3dContext->ClearRenderTargetView(
		m_renderTargetView.Get(),
		midnightBlue
		);

	m_d3dContext->ClearDepthStencilView(
		m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH,
		1.0f,
		0
		);

	// 仅在加载时绘制多维数据集(加载为异步过程)。
	if (!m_loadingComplete)
	{
		return;
	}

	m_d3dContext->OMSetRenderTargets(
		1,
		m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get()
		);

	m_d3dContext->UpdateSubresource(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0
		);

	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	m_d3dContext->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT,
		0
		);

	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	m_d3dContext->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
		);

	m_d3dContext->VSSetConstantBuffers(
		0,
		1,
		m_constantBuffer.GetAddressOf()
		);

	m_d3dContext->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
		);

	m_d3dContext->DrawIndexed(
		m_indexCount,
		0,
		0
		);
}
