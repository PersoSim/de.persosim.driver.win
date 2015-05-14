del /s /q output
for /d %%f in (output\*) do rmdir /s /q "%%f"

"%PATH_WIX%candle.exe" -trace -ext WixIIsExtension -ext WixUIExtension -dRootCertPath="..\%CERT_ROOT%" -v *.wxs -o output\
"%PATH_WIX%light.exe" output\*.wixobj -ext WixIIsExtension -ext WixUIExtension -cultures:en-us -out output\PersoSim_Test_Certificates.msi
"%EXE_SIGNTOOL%" sign /f "..\%CERT_SIGN%" /p "%CERT_SIGN_PWD%" /d "HJP Development Test Certificates" /v output\PersoSim_Test_Certificates.msi