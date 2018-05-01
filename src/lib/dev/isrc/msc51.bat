cl /I..\..\..\include /c isrc.c >> ..\..\..\msc.out
cl /I..\..\..\include /c isrcacct.c >> ..\..\..\msc.out
cl /I..\..\..\include /c isrcacld.c >> ..\..\..\msc.out
cl /I..\..\..\include /c isrcask.c >> ..\..\..\msc.out
cl /I..\..\..\include /c isrcdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c isrcdest.c >> ..\..\..\msc.out
cl /I..\..\..\include /c isrcload.c >> ..\..\..\msc.out
cl /I..\..\..\include /c isrcmdel.c >> ..\..\..\msc.out
cl /I..\..\..\include /c isrcpar.c >> ..\..\..\msc.out
cl /I..\..\..\include /c isrctemp.c >> ..\..\..\msc.out
lib ..\..\dev1.lib @response.lib >> ..\..\..\msc.out
