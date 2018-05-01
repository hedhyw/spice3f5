cl /I..\..\..\include /c mos6.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6ask.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6conv.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6dest.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6ic.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6load.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6mpar.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6par.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6set.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6temp.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6trun.c >> ..\..\..\msc.out
cl /I..\..\..\include /c mos6mask.c >> ..\..\..\msc.out
lib ..\..\dev2.lib @response.lib >> ..\..\..\msc.out
