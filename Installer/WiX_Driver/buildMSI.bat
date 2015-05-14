@ECHO off
IF "%1"=="x86" GOTO BuildForX86
IF "%1"=="x64" GOTO BuildForX64
IF "%1"=="" GOTO BuildForX86

GOTO Error

del /s /q output
for /d %%f in (output\*.*) do rmdir /s /q "%%f"
del /s /q workspace
for /d %%f in (workspace\*) do rmdir /s /q "%%f"

:BuildForX86
echo "Building driver package for x86"
set "PLATFORM_NAME=Win_7_8_x86"
set "PLATFORM_INF2CAT=8_X86,7_X86"
set "PLATFORM_WIX_SYSTEM=Win_7_8"
set "PLATFORM_WIX_ARCH=x86"
call buildPlatform.bat
IF "%1"=="" GOTO BuildForX64
IF "%1"=="x86" GOTO Done
:BuildForX64
echo "Building driver package for x64"
set "PLATFORM_NAME=Win_7_8_x64"
set "PLATFORM_INF2CAT=8_X64,7_X64"
set "PLATFORM_WIX_SYSTEM=Win_7_8"
set "PLATFORM_WIX_ARCH=x64"
call buildPlatform.bat
GOTO Done

:Error
echo entered wrong Parameter: only x86, x64 or no parameter allowed
GOTO Done

:Done