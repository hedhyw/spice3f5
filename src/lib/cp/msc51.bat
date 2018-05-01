del ..\cp.lib
cl /I..\..\include /c alias.c >> ..\..\msc.out
cl /I..\..\include /c backq.c >> ..\..\msc.out
cl /I..\..\include /c complete.c >> ..\..\msc.out
cl /I..\..\include /c cshpar.c >> ..\..\msc.out
cl /I..\..\include /c front.c >> ..\..\msc.out
cl /I..\..\include /c glob.c >> ..\..\msc.out
cl /I..\..\include /c history.c >> ..\..\msc.out
cl /I..\..\include /c input.c >> ..\..\msc.out
cl /I..\..\include /c lexical.c >> ..\..\msc.out
cl /I..\..\include /c modify.c >> ..\..\msc.out
cl /I..\..\include /c output.c >> ..\..\msc.out
cl /I..\..\include /c quote.c >> ..\..\msc.out
cl /I..\..\include /c spawn.c >> ..\..\msc.out
cl /I..\..\include /c std.c >> ..\..\msc.out
cl /I..\..\include /c unixcom.c >> ..\..\msc.out
cl /I..\..\include /c variable.c >> ..\..\msc.out
cl /I..\..\include /c var2.c >> ..\..\msc.out
cl /I..\..\include /c wlist.c >> ..\..\msc.out
cl /I..\..\include /c numparse.c >> ..\..\msc.out
lib ..\cp.lib @response.lib >> ..\..\msc.out
