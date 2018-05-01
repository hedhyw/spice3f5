cl /I..\..\..\include /c vcvs.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvsask.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvsdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvsdest.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvsfbr.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvsload.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvsmdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvspar.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvspzld.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvssacl.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvsset.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvssld.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvssprt.c >> ..\..\..\msc.out
cl /I..\..\..\include /c vcvssset.c >> ..\..\..\msc.out
lib ..\..\dev1.lib @response.lib >> ..\..\..\msc.out
