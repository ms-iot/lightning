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
del Microsoft.IoT.NativeWiring*.nupkg 2> NUL
rmdir /s /q nupkg 2> NUL

:: if a clean was requested, exit here
if "%CLEAN%"=="1" goto end

echo.
echo Creating nupkg directory structure
md nupkg
md nupkg\build
md nupkg\build\native
md nupkg\build\native\include
md nupkg\build\native\include\avr
md nupkg\build\native\source
md nupkg\build\native\lib

echo.
echo Copying files into nuget package structure
copy Microsoft.IoT.NativeWiring.nuspec nupkg /y || goto err
copy Microsoft.IoT.NativeWiring.targets nupkg\build\native /y || goto err
copy *.h nupkg\build\native\include /y || goto err
copy *.cpp nupkg\build\native\source /y || goto err
copy avr\pgmspace.h nupkg\build\native\include\avr /y || goto err
copy avr\interrupt.h nupkg\build\native\include\avr /y || goto err
copy avr\macros.h nupkg\build\native\include\avr /y || goto err
copy avr\pgmspace.h nupkg\build\native\include\avr /y || goto err
copy *.cpp nupkg\build\native\source /y || goto err
copy ..\license.txt nupkg /y || goto err

:: skip packaging step if requested
if "%NOPACK%"=="1" goto end

echo Creating NuGet Package
nuget help > NUL
IF ERRORLEVEL 1 (
    echo Please install nuget.exe from http://nuget.org
    goto err
)
nuget pack nupkg\Microsoft.IoT.NativeWiring.nuspec || goto err


:end

echo Success
exit /b 0

:err
  echo Script failed!
  exit /b 1
