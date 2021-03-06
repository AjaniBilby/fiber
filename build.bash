#!/bin/bash

Command="clang++ code/main.cpp -std=c++11 -pthread -o fiber.out"

echo "Ensure memory safety? (y/n)"
read MemSafe

if [ "$MemSafe" = "y" ]; then
  Command="$Command -D MemorySafe"
fi


echo

echo Settings;
if [ "$MemSafe" = "y" ]; then
  echo  - Memory Safety : Yes
else
  echo  - Memory Safety : No
fi
echo  - 64bit         : Yes
echo  - OS            : Native

echo
echo $Command

eval $Command
