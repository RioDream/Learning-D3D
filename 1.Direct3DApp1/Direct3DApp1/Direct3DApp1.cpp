#include "pch.h"
#include "Direct3DApp1.h"
#include "BasicTimer.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace concurrency;

//#4 构造函数，那么Direct3DApp1是个什么呢?
Direct3DApp1::Direct3DApp1() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

//#7 Initialize为一系列基本事件添加了事件处理程序，并创建了一个CubeRender对象。这个方法为应用配置好基本功能，关联的事件也与应用生命周期相关，感觉这一步属于打基础。
void Direct3DApp1::Initialize(CoreApplicationView^ applicationView)
{
	applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &Direct3DApp1::OnActivated);

	CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &Direct3DApp1::OnSuspending);

	CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &Direct3DApp1::OnResuming);

	m_renderer = ref new CubeRenderer();
}

//#8 SetWindow为基本操作事件添加事件处理程序，这些处理与窗口的显示相关，感觉属于应用搭建完成，准备显示的阶段。
void Direct3DApp1::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &Direct3DApp1::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &Direct3DApp1::OnVisibilityChanged);

	window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &Direct3DApp1::OnWindowClosed);

	window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &Direct3DApp1::OnPointerPressed);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &Direct3DApp1::OnPointerMoved);

	m_renderer->Initialize(CoreWindow::GetForCurrentThread());
}

 
void Direct3DApp1::Load(Platform::String^ entryPoint)
{
}

//#9 Run是应用的主循环，可以看到，在各项准备工作完成后，它控制整个应用的流程和消息分配，实际显示内容由CubeRenderer定义。
void Direct3DApp1::Run()
{
	BasicTimer^ timer = ref new BasicTimer();

	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			timer->Update();
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			m_renderer->Update(timer->Total, timer->Delta);
			m_renderer->Render();
			m_renderer->Present(); // 此调用将同步为显示帧速率。
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void Direct3DApp1::Uninitialize()
{
}

void Direct3DApp1::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_renderer->UpdateForWindowSizeChange();
}

void Direct3DApp1::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void Direct3DApp1::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

void Direct3DApp1::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	// 在此处插入代码。
}

void Direct3DApp1::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
	// 在此处插入代码。
}

void Direct3DApp1::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void Direct3DApp1::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// 在请求延期后异步保存应用程序状态。保留延期
	// 表示应用程序正忙于执行挂起操作。
	// 请注意，延期不是无限期的。在大约五秒后，
	// 将强制应用程序退出。
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// 在此处插入代码。

		deferral->Complete();
	});
}
 
void Direct3DApp1::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// 还原在挂起时卸载的任何数据或状态。默认情况下，
	// 在从挂起中恢复时，数据和状态会持续保留。请注意，
	// 如果之前已终止应用程序，则不会发生此事件。
}

//#3 CreateView（）接口的实现，就是创建了一个 Direct3DApp1对象
IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new Direct3DApp1();
}


//#1 主函数
[Platform::MTAThread] //表示多线程什么的
int main(Platform::Array<Platform::String^>^)
{
	//新建一个Direct3DApplicationSource对象然后执行它
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);

	return 0;
}
