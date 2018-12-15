echo off


rem If this is a debug purpose build
if /I "%1"=="debug" (
	shift

	echo Generating debugging build...
	echo.

	set Command=clang++ -g -gcodeview "./source/main.cpp" -o "./fiber.exe" -std=c++14 -Xclang -flto-visibility-public-std -fsanitize=address -O0
	set Optimize=n
	set MemSafe=y
	set OS=Native

	goto display
)




rem Use first arg as first question response
set MemSafe=%1
shift
set Optimize=%1
shift
set OS=%1
shift
set Debug=%1
shift

rem Construct the command as the build command is configured
set Command=clang++ "./source/main.cpp" -o "fiber.exe" -std=c++14 -Xclang -flto-visibility-public-std








if /I "%Optimize%"=="y" (
	set Command=%Command% -O2
) else (
	set Command=%Command% -O0
)

if /I "%MemSafe%"=="y" (
	set Command=%Command% -D MemorySafe=true
)

rem Default natic

if /I "%OS%" == "" (
	set OS=Native
	goto FinishOSSelection
)

if /I "%OS%"=="windows" (
	set Command=%Command% -target x86_64-pc-windows-msvc
	goto FinishOSSelection
)

if /I "%OS%"=="mac" (
	set Command=%Command% -target x86_64-apple-darwin-msvc
	goto FinishOSSelection
)

if /I "%OS%"=="linux" (
	set Command=%Command% -target x86_64-pc-linux-msvc
	goto FinishOSSelection
)

echo Error: Unknown OS '%OS%', presuming native
echo.
set OS=Native
:FinishOSSelection





:display
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
IF /I "%Debug%"=="y" (
	echo  - Debuging      : Yes
) ELSE (
	echo  - Debuging      : No
)
echo  - 64bit         : Yes
echo  - OS            : %OS%



:compile
echo.
echo %Command%
%Command% %1
