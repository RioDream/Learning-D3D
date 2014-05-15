#start with the D3D in windows phone app


In it's default state the solution has two projects:

[Nokia Developer:Windows Phone Direct3D XAML Application Introduction](http://developer.nokia.com/community/wiki/Windows_Phone_Direct3D_XAML_Application_Introduction)

* Direct3DXamlApp2: 
A XAML/C# project similar to a normal Windows Phone Application project. It has all the .xaml ui design files and the .cs files corresponding to those.

* Direct3DXamlApp2Comp: 
Contains the Direct3D C++ files needed to set up and render Direct3D graphics. The contents are similar to the Native C++ Direct3D template contents.
The basic functionality between these two projects is that the -Comp project code creates a DrawingSurface object which can then be used as an XAML element in the XAML/C# project and can be modified the same way (for example resized and applied to a grid) as a normal XAML UI element. Other XAML elements, such as buttons and texts can be placed on top of the DrawingSurface 


##学习
1. 
	PhoneDirect3DXamlAppComponent主文件，创建了一个 Direct3DBackground 继承于 IDrawingSurfaceManipulationHandler接口，用于处理各种交互事件，其有一个 private的变量是 cubeRenderer
	（其中要使用 CX/C++语法，才能够在C#中调用）

2. 
	CubeRenderer继承自 Direct3DBase，用于cube的 render 和 update

3. 
	Direct3DBase是一个基类，用于被cubeRenderer继承


4. 
	CreateContentProvider继承于RuntimeClass
	在irect3DBackground::CreateContentProvider()被调用



##经验
1. 

	Direct3DBackground 和 Direct3DInterop, 貌似Direct3DInterop是之前的叫法，不过我们可由此判断background是用来处理交互的。

2. 
	头文件中定义 

		XMFLOAT3* cubeColors，

	然后在cpp中使用的时候，语法没有错误，但是编译的时候遇到了unclared indentifier，这是如果将明明空间加上，
	
		DirectX::XMFLOAT3* cubeColors;

	就可以了。

