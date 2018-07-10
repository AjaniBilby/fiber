echo off
rem -std=c++11




rem Use first arg as first question response
set MemSafe=%1
shift

rem Construct the command as the build command is configured
set Command=clang++ "./code/main.cpp" -o "./fiber.exe" -std=c++14




if /I "%MemSafe%"=="y" goto SetMemSafe
if /I "%MemSafe%"=="n" goto end
:QuestionMemSafe
set MemSafe=
set /P MemSafe=Build with memory safety? (y/n) %=%

if /I "%MemSafe%"=="y" goto SetMemSafe
if /I "%MemSafe%"=="n" goto end

echo Invalid input "%MemSafe%"
goto QuestionMemSafe


:SetMemSafe
set Command=%Command% -D MemorySafe
goto end





:end
echo Settings;
IF /I "%MemSafe%"=="y" (
  echo  - Memory Safety : Yes
) ELSE (
  echo  - Memory Safety : No
)
echo  - 64bit         : Yes
echo  - OS            : Native

echo.
echo %Command%
%Command%
