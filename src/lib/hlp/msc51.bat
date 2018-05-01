del ..\hlp.lib
cl /I..\..\include /c help.c >> ..\..\msc.out
cl /I..\..\include /c provide.c >> ..\..\msc.out
cl /I..\..\include /c readhelp.c >> ..\..\msc.out
cl /I..\..\include /c textdisp.c >> ..\..\msc.out
cl /I..\..\include /c x11disp.c >> ..\..\msc.out
cl /I..\..\include /c xdisplay.c >> ..\..\msc.out
lib ..\hlp.lib @response.lib >> ..\..\msc.out
