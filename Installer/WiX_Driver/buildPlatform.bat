xcopy /s /e /y "..\input\%PLATFORM_NAME%" "workspace\%PLATFORM_NAME%\"

"%EXE_SIGNTOOL%" sign /f "..\%CERT_SIGN%" /p "%CERT_SIGN_PWD%" /d "PersoSim Virtual Reader" /v "workspace\%PLATFORM_NAME%\*.dll"

"%EXE_INF2CAT%" "/driver:workspace\%PLATFORM_NAME%" "/os:%PLATFORM_INF2CAT%"

"%EXE_SIGNTOOL%" sign /f "..\%CERT_SIGN%" /p "%CERT_SIGN_PWD%" /d "PersoSim Virtual Reader" /v "workspace\%PLATFORM_NAME%\*.cat"

"%PATH_WIX%candle.exe" -trace -ext WiXUtilExtension -ext WixDifxAppExtension.dll -ext WixIIsExtension -ext WixUIExtension -v *.wxs -dSystem="%PLATFORM_WIX_SYSTEM%" -arch "%PLATFORM_WIX_ARCH%" -o "output\%PLATFORM_NAME%\\"

"%PATH_WIX%light.exe" "output\%PLATFORM_NAME%\*.wixobj" -ext WiXUtilExtension -ext WixIIsExtension -ext WixDifxAppExtension.dll -ext WixUIExtension -cultures:en-us "%PATH_WIX%difxapp_%PLATFORM_WIX_ARCH%.wixlib" -out "output\PersoSim_%PLATFORM_NAME%.msi"
"%EXE_SIGNTOOL%" sign /f "..\%CERT_SIGN%" /p "%CERT_SIGN_PWD%" /d "PersoSim Virtual Reader Driver" /v "output\PersoSim_%PLATFORM_NAME%.msi"