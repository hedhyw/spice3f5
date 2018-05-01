del ..\misc.lib
cl /I..\..\include /c alloc.c >> ..\..\msc.out
cl /I..\..\include /c dos_dirs.c >> ..\..\msc.out
cl /I..\..\include /c dup2.c >> ..\..\msc.out
cl /I..\..\include /c ivars.c >> ..\..\msc.out
cl /I..\..\include /c math.c >> ..\..\msc.out
cl /I..\..\include /c mktemp.c >> ..\..\msc.out
cl /I..\..\include /c printnum.c >> ..\..\msc.out
cl /I..\..\include /c string.c >> ..\..\msc.out
cl /I..\..\include /c tilde.c >> ..\..\msc.out
cl /I..\..\include /c time.c >> ..\..\msc.out
lib ..\misc.lib @response.lib >> ..\..\msc.out
