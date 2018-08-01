:: Ring - native Windows LRC project generator

@echo off
setlocal

if "%1" == "/?" goto Usage
if "%~1" == "" goto Usage

set doGen=N
set doBuild=N

set SCRIPTNAME=%~nx0

if "%1"=="gen" (
    set doGen=Y
) else if "%1"=="build" (
    set doBuild=Y
) else (
    goto Usage
)

set arch=N

shift
:ParseArgs
if "%1" == "" goto FinishedArgs
if /I "%1"=="x86" (
    set arch=x86
) else if /I "%1"=="x64" (
    set arch=x64
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
    set Comp_x86=x86 10.0.15063.0
    set Comp_x64=x86_amd64 10.0.15063.0
) else (
    set Comp_x86=amd64_x86 10.0.15063.0
    set Comp_x64=amd64 10.0.15063.0
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
    set CMAKE_OPTIONS=-DQt5Core_DIR=!QtCmakeDir!\\Qt5Core -DQt5Sql_DIR=!QtCmakeDir!\\Qt5Sql -DQt5LinguistTools_DIR=!QtCmakeDir!\\Qt5LinguistTools -DQt5Concurrent_DIR=!QtCmakeDir!\\Qt5Concurrent -Dring_BIN=!DaemonDir!\MSVC\x86\ReleaseLib_win32\bin\dring.lib -DRING_INCLUDE_DIR=!DaemonDir!\src\dring
) else if "%arch%"=="x64" (
    set CMAKE_GENERATOR_STRING="Visual Studio 15 2017 Win64"
    set QtCmakeDir=%QtDir%\\msvc2017_64\\lib\\cmake
    set CMAKE_OPTIONS=-DQt5Core_DIR=!QtCmakeDir!\\Qt5Core -DQt5Sql_DIR=!QtCmakeDir!\\Qt5Sql -DQt5LinguistTools_DIR=!QtCmakeDir!\\Qt5LinguistTools -DQt5Concurrent_DIR=!QtCmakeDir!\\Qt5Concurrent -Dring_BIN=!DaemonDir!\MSVC\x64\ReleaseLib_win32\bin\dring.lib -DRING_INCLUDE_DIR=!DaemonDir!\src\dring
)
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
echo     %0 [action] [architecture]
echo:
echo where
echo:
echo [action]           is: gen   ^| build
echo [architecture]     is: x86   ^| x64
echo:
echo For example:
echo     %SCRIPTNAME% gen x86     - gen x86 static lib vs projects for qtwrapper/lrc
echo     %SCRIPTNAME% build x64   - build x86 qtwrapper/lrc static libs
echo:
goto :eof

:cleanup
endlocal
exit /B %ERRORLEVEL%