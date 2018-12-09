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
	gate,            // if
	gateOther,       // else
	compare,         // cmpr
	bitwise,         // bit
	initilize,       // init
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
