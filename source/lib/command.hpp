#pragma once

#include <string>

enum class ComparisonOpperators: unsigned short{
	greater,
	lesser,
	equal
};

enum class Command: unsigned short{
	invalid,
	stop,            // exit
	jump,            // jump
	set,             // set
	mode,            // mode
	math,            // math
	gate,            // if
	gateOther,       // else
	compare,         // cmpr
	bitwise,         // bit
	initilize,       // init
	bind,            // bind
	execute,         // exec
	blockExit,       // break
	blockRepeat,     // continue
	blockOpen,       // {
	blockClose,      // }
	standardStream,  // ss
	memalloc,        // malloc
	function,        // func
	rtrn,            // return
};

#include "./command.cpp"
