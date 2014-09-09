@REM @file
@REM   Windows batch file to build the tree for the MinnowBoard.
@REM
@REM Copyright (c) 2012 - 2013, Intel Corporation. All rights reserved.<BR>
@REM This program and the accompanying materials
@REM are licensed and made available under the terms and conditions of the BSD License
@REM which accompanies this distribution.  The full text of the license may be found at
@REM http://opensource.org/licenses/bsd-license.php
@REM
@REM THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
@REM WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
@REM

@if "" == "%echo_on%" (
  @echo off
)
if /I "%1"=="/?" goto Usage

if not defined WORKSPACE (
  call edksetup.bat
  @if "" == "%echo_on%" (
    @echo off
  )
)
set Package=MinnowBoardIntelRuPkg

REM
REM Choose a default build number
REM

set BuildDate=
set BuildTime=
set BuildYear=
set BuildNumber=
set ParseDate=
set ParseTime=
set BuildDateTime= %DATE% %TIME%
FOR /F "tokens=1-3 delims== " %%I IN ("%BuildDateTime%") DO (
  set ParseDate=%%J
  set ParseTime=%%K
)
FOR /F "tokens=1-3 delims==/" %%W IN ("%ParseDate%") DO (
  set /a BuildYear = %%Y - 2000
  set BuildDate=%%W%%X
)
FOR /F "tokens=1-3 delims==:" %%L IN ("%ParseTime%") DO (
  set BuildTime=%%L%%M
)
if %BuildTime% GEQ 1000 (
  set BuildNumber=%BuildYear%%BuildDate%%BuildTime%
) else (
  set BuildNumber=%BuildYear%%BuildDate%0%BuildTime%
)
set BuildDate=%ParseDate%
set BuildDateTime=
set BuildTime=
set BuildYear=
set ParseDate=
set ParseTime=
echo BuildNumber: %BuildNumber%

REM
REM Select the build options
REM

set Options= -a IA32 -D BUILD_NUMBER=%BuildNumber% -D BUILD_DATE=%BuildDate% -D INCLUDE_DP=TRUE
set DebugOptions= -D LOGGING=TRUE -D SYMBOLIC_DEBUG=TRUE

if /I "%1"=="-r32" goto ReleaseBuild32
if /I "%1"=="-r32_1" goto ReleaseBuild32_1
if /I "%1"=="-d32" goto DebugBuild32
if /I "%1"=="-d32_1" goto DebugBuild32_1
if /I "%1"=="-clean" goto Cleanup
if /I "%1"=="" (
  goto DebugBuild32
) else (
  echo.
  echo  Error: "%1" is not valid parameter.
  goto Usage
)


:ReleaseBuild32
set Options=-n 8 %Options%

:ReleaseBuild32_1
set DebugOptions=
set Options=%Options%%DebugOptions%
echo on
build -p %Package%\Platform.dsc -b RELEASE %Options%
@if errorlevel 1 (

  @set ERR_VALUE=%ERRORLEVEL%

  @goto ErrorFound

)

@goto End

:DebugBuild32
set Options=-n 8 %Options%

:DebugBuild32_1
set Options=%Options%%DebugOptions%
echo on
build -p %Package%\Platform.dsc -b DEBUG %Options%
@if errorlevel 1 (
  @set ERR_VALUE=%ERRORLEVEL%
  @goto ErrorFound
)
@goto End

:Cleanup
if exist Build rmdir Build /s /q
if exist Conf\.cache rmdir Conf\.cache /s /q
del Conf\FrameworkDatabase.db /q /f
goto End


:ErrorFound
@if "" == "%echo_on%" (
  @echo off
)
echo .
echo on
exit /B %ERR_VALUE%


:Usage

echo.
echo  Usage: "%0 [/? | -r32 | -r32_1|  -d32 |  -d32_1 | -clean] [-binary]"
echo.


:End

@if "" == "%echo_on%" (
  @echo off
)
echo.
echo on

