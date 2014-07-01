@echo off
setlocal enableextensions disabledelayedexpansion

:: Parse options
:GETOPTS
 if /I "%~1" == "/?" goto USAGE
 if /I "%~1" == "/Help" goto USAGE
 if /I "%~1" == "/clean" set CLEAN=1
 if /I "%~1" == "/nopack" set NOPACK=1
 shift
if not (%1)==() goto GETOPTS

echo Cleaning outputs
del Microsoft.IoT.Galileo.Arduino*.nupkg 2> NUL
rmdir /s /q nupkg 2> NUL

:: if a clean was requested, exit here
if "%CLEAN%"=="1" goto end

echo.
echo Creating nupkg directory structure
md nupkg
md nupkg\build
md nupkg\build\native
md nupkg\build\native\include
md nupkg\build\native\source
md nupkg\build\native\lib

echo.
echo Copying files into nuget package structure
copy Microsoft.IoT.Galileo.Arduino.nuspec nupkg /y || goto err
copy Microsoft.IoT.Galileo.Arduino.targets nupkg\build\native /y || goto err
copy *.h nupkg\build\native\include /y || goto err
copy *.cpp nupkg\build\native\source /y || goto err
copy ..\extern\inc\embprpusr.h nupkg\build\native\include /y || goto err
copy ..\extern\inc\galileo.h nupkg\build\native\include /y || goto err
copy ..\extern\lib\x86\embprpusr.lib nupkg\build\native\lib /y || goto err
copy license.txt nupkg /y || goto err

:: skip packaging step if requested
if "%NOPACK%"=="1" goto end

echo Creating NuGet Package
nuget help > NUL
IF ERRORLEVEL 1 (
    echo Please install nuget.exe from http://nuget.org
    goto err
)
nuget pack nupkg\Microsoft.IoT.Galileo.Arduino.nuspec || goto err


:end

echo Success
exit /b 0

:err
  echo Script failed!
  exit /b 1
