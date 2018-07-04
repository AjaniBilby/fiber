echo off
rem -std=c++11


:question
set INPUT=
set /P INPUT=Build with memory safety? (y/n) %=%

if /I "%INPUT%"=="y" goto safe
if /I "%INPUT%"=="n" goto unsafe

echo Invalid input "%INPUT%"
goto question



:unsafe
clang++ "./code/main.cpp" -o "./fiber.exe"
goto end


:safe
clang++ -D MemorySafe "./code/main.cpp" -o "./fiber.exe"
goto end

:end