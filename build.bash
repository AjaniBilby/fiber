#!/bin/bash

Command="clang++ source/main.cpp -std=c++14 -pthread -o fiber.out"



# Gather build settings
echo "Ensure memory safety? (y/n)"
read MemSafe
if [ "$MemSafe" = "y" ]; then
	Command="$Command -D MemorySafe=true"
fi
echo "Optimized build? (y/n)"
read Optimize
if [ "$Optimize" = "y" ]; then
	Command="$Command -O2"
fi
echo "Target Platform? (windows/mac/linux)"
read OS
if ["$OS" = ""]; then
	OS = "linux"
fi
if [ "$OS" = "win" ]; then
	Command="$Command -target x86_64-pc-windows-msvc"
else if ["$OS" = "mac"]; then
	Command="$Command -target x86_64-apple-darwin-msvc"
else if ["$OS" = "linux"]; then
	Command="$Command  -target x86_64-pc-linux-msvc"
fi



echo

echo Settings;
if [ "$MemSafe" = "y" ]; then
	echo  - Memory Safety : Yes
else
	echo  - Memory Safety : No
fi
if [ "$Optimize" = "y" ]; then
	echo  - Optimized     : Yes
else
	echo  - Optimized     : No
fi
echo  - OS            : $OS
echo  - 64bit         : Yes

echo
echo $Command

eval $Command
