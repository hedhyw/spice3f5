cl /I..\..\..\include /c sw.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swacload.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swask.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swdelete.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swdest.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swload.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swmask.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swmdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swmparam.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swnoise.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swparam.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swpzload.c >> ..\..\..\msc.out
cl /I..\..\..\include /c swsetup.c >> ..\..\..\msc.out
lib ..\..\dev3.lib @response.lib >> ..\..\..\msc.out
