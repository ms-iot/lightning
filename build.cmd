@echo off

cd %~dp0

if /i "%1"=="help" goto help
if /i "%1"=="--help" goto help
if /i "%1"=="-help" goto help
if /i "%1"=="/help" goto help
if /i "%1"=="?" goto help
if /i "%1"=="-?" goto help
if /i "%1"=="--?" goto help
if /i "%1"=="/?" goto help

@rem Process arguments.
set config=
set target=Build
set noprojgen=
set nobuild=
set run=
set vs_toolset=x86


:next-arg
if "%1"=="" goto args-done
if /i "%1"=="debug"        set config=Debug&goto arg-ok
if /i "%1"=="release"      set config=Release&goto arg-ok
if /i "%1"=="clean"        set target=Clean&goto arg-ok
if /i "%1"=="nobuild"      set nobuild=1&goto arg-ok
:arg-ok
shift
goto next-arg
:args-done

if defined WindowsSDKDir goto select-target
if defined VCINSTALLDIR goto select-target

@rem Look for Visual Studio 2015
if not defined VS140COMNTOOLS goto vc-set-2013
if not exist "%VS140COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2013
call "%VS140COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2015
echo Using Visual Studio 2015
goto select-target

:vc-set-2013
@rem Look for Visual Studio 2013
if not defined VS120COMNTOOLS goto vc-set-2012
if not exist "%VS120COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2012
call "%VS120COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2013
echo Using Visual Studio 2013
goto select-target

:vc-set-2012
@rem Look for Visual Studio 2012
if not defined VS110COMNTOOLS goto vc-set-2010
if not exist "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2010
call "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2012
echo Using Visual Studio 2012
goto select-target

:vc-set-2010
@rem Look for Visual Studio 2010
if not defined VS100COMNTOOLS goto vc-set-2008
if not exist "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-2008
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2010
echo Using Visual Studio 2010
goto select-target

:vc-set-2008
@rem Look for Visual Studio 2008
if not defined VS90COMNTOOLS goto vc-set-notfound
if not exist "%VS90COMNTOOLS%\..\..\vc\vcvarsall.bat" goto vc-set-notfound
call "%VS90COMNTOOLS%\..\..\vc\vcvarsall.bat" %vs_toolset%
set GYP_MSVS_VERSION=2008
echo Using Visual Studio 2008
goto select-target

:vc-set-notfound
echo Warning: Visual Studio not found

:select-target
if not "%config%"=="" goto msbuild
set config=Release

:msbuild
@rem Skip project generation if requested.
if defined nobuild goto pack

@rem Check if VS build env is available
if defined VCINSTALLDIR goto msbuild-found
if defined WindowsSDKDir goto msbuild-found
echo Build skipped. To build, this file needs to run from VS cmd prompt.
goto run

@rem Build the sln with msbuild.
:msbuild-found
msbuild Library\Lightning.sln /m /t:%target% /p:Configuration=%config% /p:Platform=ARM /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo
if errorlevel 1 exit /b 1
msbuild Library\Lightning.sln /m /t:%target% /p:Configuration=%config% /p:Platform=x86 /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo
if errorlevel 1 exit /b 1
msbuild Library\Lightning.sln /m /t:%target% /p:Configuration=%config% /p:Platform=x64 /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo
if errorlevel 1 exit /b 1

:pack
cd Nuget

if not "%config%"=="Debug" goto releasebuild
CALL build-nupkg.cmd /debug
cd ..
goto exit

:releasebuild
CALL build-nupkg.cmd
cd ..
goto :exit

:help
echo build.bat [debug/release] [clean] [nobuild]
echo Examples:
echo   build.bat               : builds release build
echo   build.bat debug         : builds debug build
echo   build.bat nobuild       : Create nuget only
goto exit

:exit
