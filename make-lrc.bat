:: Ring - native Windows LRC project generator

@echo off
if "%1" == "/?" goto Usage
if "%~1" == "" goto Usage

set doGen=N
set doBuild=N

set SCRIPTNAME=%~nx0

if "%1"=="gen" (
    set doGen=Y
    set command=Generate
) else if "%1"=="build" (
    set doBuild=Y
    set command=Build
) else (
    goto Usage
)

if "%doGen%"=="Y" (
    if NOT "%4"=="" goto Version_New
    goto Default_version
) else (
    goto StartLocal
)

:Version_New
set QtDir=C:\\Qt\\%4%
set Version=%4%
if not exist "%QtDir%" echo This Qt path does not exist, using default version.
if exist "%QtDir%" goto StartLocal
:Default_version
set QtDir=C:\\Qt\\5.9.4
set Version=5.9.4
if not exist "%QtDir%" echo Default Qt path does not exist, check your installation path. &goto Usage

setlocal
:StartLocal

set arch=N

shift
:ParseArgs
if "%1" == "" goto FinishedArgs
if /I "%1"=="x86" (
    set arch=x86
) else if /I "%1"=="x64" (
    set arch=x64
) else if /I "%1"=="version" (
    shift
) else (
    goto Usage
)
shift
goto ParseArgs

:FinishedArgs
if "%arch%"=="x86" (
    set MSBUILD_ARGS=/nologo /p:useenv=true /p:Configuration=Release /p:Platform=Win32 /verbosity:normal /maxcpucount:%NUMBER_OF_PROCESSORS%
) else if "%arch%"=="x64" (
    set MSBUILD_ARGS=/nologo /p:useenv=true /p:Configuration=Release /p:Platform=x64 /verbosity:normal /maxcpucount:%NUMBER_OF_PROCESSORS%
)

@setlocal

set VSInstallerFolder="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
if %PROCESSOR_ARCHITECTURE%==x86 set VSInstallerFolder="%ProgramFiles%\Microsoft Visual Studio\Installer"

pushd %VSInstallerFolder%
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set VSLATESTDIR=%%i
)
popd

echo VS Installation folder: %VSLATESTDIR%

if not exist "%VSLATESTDIR%\VC\Auxiliary\Build\vcvarsall.bat" (
    echo:
    echo VSInstallDir not found or not installed correctly.
    goto cleanup
)

if %PROCESSOR_ARCHITECTURE%==x86 (
    set Comp_x86=x86 10.0.16299.0
    set Comp_x64=x86_amd64 10.0.16299.0
) else (
    set Comp_x86=amd64_x86 10.0.16299.0
    set Comp_x64=amd64 10.0.16299.0
)

set path=%path:"=%
if "%arch%"=="x86" (
    call "%VSLATESTDIR%"\\VC\\Auxiliary\\Build\\vcvarsall.bat %Comp_x86%
) else if "%arch%"=="x64" (
    call "%VSLATESTDIR%"\\VC\\Auxiliary\\Build\\vcvarsall.bat %Comp_x64%
)

if "%arch%" neq "N" (
    if "%doGen%" neq "N" (
        goto genLRC
    ) else if "%doBuild%" neq "N" (
		goto buildLRC
    )
    goto :eof
)
goto Usage

:genLRC
echo [96mGenerating using Qt version: %Version%[0m
setlocal EnableDelayedExpansion
set DaemonDir=%cd%\\..\\daemon
mkdir msvc
cd msvc
set PATH=C:\\Program Files\\CMake\\bin\\;%PATH%
if "echo QtDir is: %QtDir%"=="" (
    echo Error: QtDir not specified
    goto cleanup
)
set CMAKE_GENERATOR_STRING=""
set CMAKE_OPTIONS=""
if "%arch%"=="x86" (
    set CMAKE_GENERATOR_STRING="Visual Studio 15 2017 Win32"
    set QtCmakeDir=%QtDir%\\msvc2017\\lib\\cmake
) else if "%arch%"=="x64" (
    set CMAKE_GENERATOR_STRING="Visual Studio 15 2017 Win64"
    set QtCmakeDir=%QtDir%\\msvc2017_64\\lib\\cmake
)
set CMAKE_OPTIONS=-DQt5Core_DIR=!QtCmakeDir!\\Qt5Core -DQt5Sql_DIR=!QtCmakeDir!\\Qt5Sql -DQt5LinguistTools_DIR=!QtCmakeDir!\\Qt5LinguistTools -DQt5Concurrent_DIR=!QtCmakeDir!\\Qt5Concurrent -DQt5Gui_DIR=!QtCmakeDir!\\Qt5Gui -Dring_BIN=!DaemonDir!\MSVC\x64\ReleaseLib_win32\bin\dring.lib -DRING_INCLUDE_DIR=!DaemonDir!\src\dring
cmake .. -G !CMAKE_GENERATOR_STRING! !CMAKE_OPTIONS!
endlocal
goto cleanup

:buildLRC
:: build qtwrapper
msbuild msvc\src\qtwrapper\qtwrapper.vcxproj %MSBUILD_ARGS%
:: build lrc
msbuild msvc\ringclient_static.vcxproj %MSBUILD_ARGS%
goto cleanup

@endlocal

:Usage
echo:
echo The correct usage is:
echo:
echo     %SCRIPTNAME% [action] [architecture] [version] [version_para]
echo:
echo where
echo:
echo [action]           is: gen   ^| build
echo [architecture]     is: x86   ^| x64
echo [version]          is: version - optional
echo [version_para]     is: 5.9.4 ^| Qt version installed
echo:
echo For example:
echo     %SCRIPTNAME% gen x86                    - gen x86 static lib vs projects for qtwrapper/lrc for Qt version 5.9.4
echo     %SCRIPTNAME% gen x86 version 5.12.0     - gen x86 static lib vs projects for qtwrapper/lrc for Qt version 5.12.0
echo     %SCRIPTNAME% build x64                  - build x64 qtwrapper/lrc static libs
echo:
goto :eof

:cleanup
endlocal
if %ERRORLEVEL% geq 1 (
    echo [91m%command% failed[0m
    exit %ERRORLEVEL%
) else (
    echo [92m%command% succeeded[0m
    exit /B %ERRORLEVEL%
)
