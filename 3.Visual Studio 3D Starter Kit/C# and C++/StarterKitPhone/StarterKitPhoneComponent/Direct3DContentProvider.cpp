// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "Direct3DContentProvider.h"

using namespace StarterKitPhoneComponent;

//使用background作为controller来构造 contentProvider
Direct3DContentProvider::Direct3DContentProvider(Direct3DBackground^ controller) :
    m_controller(controller)
{
	//注册RequestAdditionalFrame事件的响应函数为 m_host->RequestAdditionalFrame()
    m_controller->RequestAdditionalFrame += ref new RequestAdditionalFrameHandler([=] ()
    {
        if (m_host)
        {
            m_host->RequestAdditionalFrame();
        }
    });
}


//一下这些方法都调用 其background的同名方法
// IDrawingSurfaceContentProviderNative interface
HRESULT Direct3DContentProvider::Connect(_In_ IDrawingSurfaceRuntimeHostNative* host, _In_ ID3D11Device1* device)
{
    m_host = host;

    return m_controller->Connect(host, device);
}

void Direct3DContentProvider::Disconnect()
{
    m_controller->Disconnect();
    m_host = nullptr;
}

HRESULT Direct3DContentProvider::PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Inout_ DrawingSurfaceSizeF* desiredRenderTargetSize)
{
    return m_controller->PrepareResources(presentTargetTime, desiredRenderTargetSize);
}

HRESULT Direct3DContentProvider::Draw(_In_ ID3D11Device1* device, _In_ ID3D11DeviceContext1* context, _In_ ID3D11RenderTargetView* renderTargetView)
{
    return m_controller->Draw(device, context, renderTargetView);
}