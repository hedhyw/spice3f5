del ..\ni.lib
cl /I..\..\include /c niaciter.c >> ..\..\msc.out
cl /I..\..\include /c nicomcof.c >> ..\..\msc.out
cl /I..\..\include /c niconv.c >> ..\..\msc.out
cl /I..\..\include /c nidest.c >> ..\..\msc.out
cl /I..\..\include /c niditer.c >> ..\..\msc.out
cl /I..\..\include /c niinit.c >> ..\..\msc.out
cl /I..\..\include /c niinteg.c >> ..\..\msc.out
cl /I..\..\include /c niiter.c >> ..\..\msc.out
cl /I..\..\include /c niniter.c >> ..\..\msc.out
cl /I..\..\include /c nipred.c >> ..\..\msc.out
cl /I..\..\include /c nipzmeth.c >> ..\..\msc.out
cl /I..\..\include /c nireinit.c >> ..\..\msc.out
cl /I..\..\include /c nisenre.c >> ..\..\msc.out
lib ..\ni.lib @response.lib >> ..\..\msc.out
