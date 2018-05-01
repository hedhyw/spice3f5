cl /I..\..\..\include /c urc.c >> ..\..\..\msc.out
cl /I..\..\..\include /c urcask.c >> ..\..\..\msc.out
cl /I..\..\..\include /c urcdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c urcdest.c >> ..\..\..\msc.out
cl /I..\..\..\include /c urcmask.c >> ..\..\..\msc.out
cl /I..\..\..\include /c urcmdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c urcmpar.c >> ..\..\..\msc.out
cl /I..\..\..\include /c urcparam.c >> ..\..\..\msc.out
cl /I..\..\..\include /c urcsetup.c >> ..\..\..\msc.out
lib ..\..\dev3.lib @response.lib >> ..\..\..\msc.out
