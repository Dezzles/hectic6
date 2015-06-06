call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\vsvars32.bat"

cd Projects\vs2013-windows-vs-v120
msbuild HecticJam6.sln /p:Configuration=Release /property:Platform=x64


cd ..\..\Dist
HecticJam6-vs2013-windows-vs-v120-Release.exe ImportPackages

copy HecticJam6-vs2013-windows-vs-v120-Release.exe ..\Deploy
xcopy PackedContent ..\Deploy\PackedContent /S /Y

pause
