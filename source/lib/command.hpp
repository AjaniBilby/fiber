#pragma once

#include <string>

enum class ComparisonOpperators: unsigned char{
	greater,
	lesser,
	equal
};

enum class Command: unsigned char{
	invalid,
	blank,           // (internal use only)
	jump,            // (internal use only)
	stop,            // stop
	set,             // set
	mode,            // mode
	math,            // math
	compare,         // cmpr
	bitwise,         // bit
	gate,            // if
	gateOther,       // else
	initilize,       // init
	loop,            // loop
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
