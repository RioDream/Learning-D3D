
PageOrientation
[PageOrientation](http://msdn.microsoft.com/zh-cn/library/system.printing.pageorientation(v=vs.110).aspx)
[DisplayOrientations](http://msdn.microsoft.com/en-us/library/windows/apps/windows.graphics.display.displayorientations.aspx)


[图解](http://www.chenhaiyun.com/1166.html)
[Dip Pixel on so 1](http://stackoverflow.com/questions/2025282/difference-between-px-dp-dip-and-sp-in-android)
[Dip Pixel on so 2](http://stackoverflow.com/questions/5591868/android-pixels-to-dips)
dp
Density-independent Pixels - an abstract unit that is based on the physical density of the screen. These units are relative to a 160 dpi screen, so one dp is one pixel on a 160 dpi screen. The ratio of dp-to-pixel will change with the screen density, but not necessarily in direct proportion. Note: The compiler accepts both "dip" and "dp", though "dp" is more consistent with "sp".

	pixels = dps * (density / 160)

px
Pixels - corresponds to actual pixels on the screen.

