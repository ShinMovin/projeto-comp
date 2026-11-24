@echo off
setlocal

if not defined MSYS2_ROOT set "MSYS2_ROOT=C:\msys64"
set "ucrt=%MSYS2_ROOT%\ucrt64\bin"
set "msys=%MSYS2_ROOT%\usr\bin"

if not exist "%ucrt%\mingw32-make.exe" goto msys2_ausente
if not exist "%ucrt%\gcc.exe" goto msys2_ausente
if not exist "%msys%\sh.exe" goto msys2_ausente

set "PATH=%ucrt%;%msys%;%PATH%"
set "shell_msys=%msys:\=/%/sh.exe"
pushd "%~dp0" || exit /b 1

"%ucrt%\mingw32-make.exe" -C codigo "SHELL=%shell_msys%" "CFLAGS=-Wall -Wextra -Werror"
set "status=%ERRORLEVEL%"
if not "%status%"=="0" goto fim
if /i not "%~1"=="jogar" goto fim

.\compilado\jogo.exe
set "status=%ERRORLEVEL%"

:fim
popd
exit /b %status%

:msys2_ausente
echo Ferramentas do MSYS2 UCRT64 ausentes em "%MSYS2_ROOT%". 1>&2
echo Se o MSYS2 estiver em outra pasta, defina MSYS2_ROOT. 1>&2
exit /b 1
