del /s /q output
for /d %%f in (output\*) do rmdir /s /q "%%f"

"%PATH_WIX%candle.exe" -ext WixBalExtension -v *.wxs -o output\
"%PATH_WIX%light.exe" -ext WixBalExtension output\*.wixobj -o output\PersoSim_Installation_Bundle.exe

rem sign the burn engine
"%PATH_WIX%insignia.exe" -ib output\PersoSim_Installation_Bundle.exe -o output\engine.exe
"%EXE_SIGNTOOL%" sign /f "..\%CERT_SIGN%" /p "%CERT_SIGN_PWD%" /d "Burn Engine" /v output\engine.exe
"%PATH_WIX%insignia" -ab output\engine.exe output\PersoSim_Installation_Bundle.exe -o output\PersoSim_Installation_Bundle.exe

rem sign the installer
"%EXE_SIGNTOOL%" sign /f "..\%CERT_SIGN%" /p "%CERT_SIGN_PWD%" /d "PersoSim Installation Bundle" /v output\PersoSim_Installation_Bundle.exe