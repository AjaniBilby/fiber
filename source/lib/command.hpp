#pragma once

#include <string>

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
	longCompare,     // lcmpr
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
