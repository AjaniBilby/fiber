echo off




rem Use first arg as first question response
set MemSafe=%1
shift
set Optimize=%1
shift

rem Construct the command as the build command is configured
set Command=clang++ "./code/main.cpp" -o "./fiber.exe" -std=c++14 -Xclang -flto-visibility-public-std









if /I "%Optimize%"=="y" goto SetOptimize
if /I "%Optimize%"=="n" goto EndQstOptmz
:QuestionDebug
set Optimize=
set /P Optimize=Optimize (longer build time)? (y/n) %=%

if /I "%Optimize%"=="y" goto SetOptimize
if /I "%Optimize%"=="n" goto EndQstOptmz

echo Invalid input "%Optimize%"
goto SetOptimize


:SetOptimize
set Command=%Command% -O2
goto EndQstOptmz

:EndQstOptmz






if /I "%MemSafe%"=="y" goto SetMemSafe
if /I "%MemSafe%"=="n" goto EndQstMemSafe
:QuestionMemSafe
set MemSafe=
set /P MemSafe=Build with memory safety? (y/n) %=%

if /I "%MemSafe%"=="y" goto SetMemSafe
if /I "%MemSafe%"=="n" goto EndQstMemSafe

echo Invalid input "%MemSafe%"
goto QuestionMemSafe


:SetMemSafe
set Command=%Command% -D MemorySafe
goto EndQstMemSafe

:EndQstMemSafe





:end
echo Settings;
IF /I "%MemSafe%"=="y" (
  echo  - Memory Safety : Yes
) ELSE (
  echo  - Memory Safety : No
)
IF /I "%Optimize%"=="y" (
  echo  - Optimized     : Yes
) ELSE (
  echo  - Optimized     : No
)
echo  - 64bit         : Yes
echo  - OS            : Native

echo.
echo %Command% %1
%Command% %1
