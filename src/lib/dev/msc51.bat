del ..\dev1.lib
del ..\dev2.lib
del ..\dev3.lib
cl /I..\..\include /c devsup.c >> ..\..\msc.out
lib ..\dev1.lib @response.lib >> ..\..\msc.out
cd disto
command /c msc51.bat
cd ..
cd asrc
command /c msc51.bat
cd ..
cd bjt
command /c msc51.bat
cd ..
cd bsim1
command /c msc51.bat
cd ..
cd bsim2
command /c msc51.bat
cd ..
cd cap
command /c msc51.bat
cd ..
cd cccs
command /c msc51.bat
cd ..
cd ccvs
command /c msc51.bat
cd ..
cd csw
command /c msc51.bat
cd ..
cd dio
command /c msc51.bat
cd ..
cd ind
command /c msc51.bat
cd ..
cd isrc
command /c msc51.bat
cd ..
cd jfet
command /c msc51.bat
cd ..
cd ltra
command /c msc51.bat
cd ..
cd mes
command /c msc51.bat
cd ..
cd mos1
command /c msc51.bat
cd ..
cd mos2
command /c msc51.bat
cd ..
cd mos3
command /c msc51.bat
cd ..
cd mos6
command /c msc51.bat
cd ..
cd res
command /c msc51.bat
cd ..
cd sw
command /c msc51.bat
cd ..
cd tra
command /c msc51.bat
cd ..
cd urc
command /c msc51.bat
cd ..
cd vccs
command /c msc51.bat
cd ..
cd vcvs
command /c msc51.bat
cd ..
cd vsrc
command /c msc51.bat
cd ..
