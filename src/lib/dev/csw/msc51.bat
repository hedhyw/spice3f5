cl /I..\..\..\include /c csw.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswacld.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswask.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswdest.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswload.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswmask.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswmdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswmpar.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswnoise.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswparam.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswpzld.c >> ..\..\..\msc.out
cl /I..\..\..\include /c cswsetup.c >> ..\..\..\msc.out
lib ..\..\dev3.lib @response.lib >> ..\..\..\msc.out
