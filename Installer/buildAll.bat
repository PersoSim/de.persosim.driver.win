call configure.bat

IF "%1"=="bundle" GOTO Bundle

cd WiX_Certificate
call buildMSI.bat %1
cd ..
cd WiX_Driver
call buildMSI.bat %1
cd ..


IF "%1"=="x86" GOTO Done
IF "%1"=="x64" GOTO Done
IF "%1"=="" GOTO Bundle

GOTO Error

:Bundle
cd WiX_Bundle
call buildBundle.bat %1
cd ..

GOTO Done

:Error
echo An invalid parameter %1 has been used
echo Possible parameters:
echo x86
echo x64
echo bundle

:Done