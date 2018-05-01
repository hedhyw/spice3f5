cl /I..\..\..\include /c vccs.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccsask.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccsdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccsdest.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccsload.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccsmdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccspar.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccspzld.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccssacl.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccsset.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccssld.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccssprt.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vccssset.c >> ..\..\..\msc.out
lib ..\..\dev1.lib @response.lib >> ..\..\..\msc.out
