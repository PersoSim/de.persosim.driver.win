@echo off
set "PATH_WIX=C:\Program Files (x86)\WiX Toolset v3.8\bin\"

set "EXE_SIGNTOOL=C:\Program Files (x86)\Windows Kits\8.1\bin\x86\signtool.exe"
set "EXE_INF2CAT=C:\Program Files (x86)\Windows Kits\8.1\bin\x86\Inf2Cat.exe"
set "CERT_ROOT=cert\PersoSim_Root_CA.cer"

rem set "CERT_SIGN=C:\workspaces\PersoSimDriverWin\Installer\cert\PersoSim_Release_Certificate.p12"
rem set /p "CERT_SIGN_PWD=Enter private key password for signing:"

set "CERT_SIGN=cert\PersoSim_Development_Certificate.p12"
set "CERT_SIGN_PWD=7I_PR3NTPHsAm93Mad0tXCBJtaNVU_dnkhLyuO-iEwDuaR-q"
