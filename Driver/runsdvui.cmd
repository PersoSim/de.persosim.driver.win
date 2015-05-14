cd /d "C:\Users\hfunke\git_repo\PersoSimDriverWin\Driver" &msbuild "PersoSimVirtualReader.vcxproj" /t:sdvViewer /p:configuration="Win7 Debug" /p:platform=Win32
exit %errorlevel% 