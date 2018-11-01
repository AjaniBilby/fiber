#include "./command.hpp"


std::string ToString(Command cmd){
	switch (cmd){
		case Command::invalid:
			return "invalid";
		case Command::function:
			return "function";
		case Command::rtrn:
			return "return";
		case Command::math:
			return "math";
		case Command::set:
			return "set";
		case Command::gate:
			return "if";
		case Command::gateOther:
			return "else";
		case Command::compare:
			return "compare";
		case Command::longCompare:
			return "longCompare";
		case Command::bitwise:
			return "bitwise";
		case Command::initilize:
			return "init";
		case Command::bind:
			return "bind";
		case Command::execute:
			return "execute";
		case Command::blockExit:
			return "break";
		case Command::blockRepeat:
			return "continue";
		case Command::blockOpen:
			return "{";
		case Command::blockClose:
			return "}";
		case Command::standardStream:
			return "ss";
		case Command::memalloc:
			return "malloc";
		case Command::mode:
			return "mode";
		case Command::stop:
			return "exit";
	}

	return "Err";
};

Command CommandFrom(std::string str){
	if (str == "exit"){
		return Command::stop;
	}
	if (str == "jump"){
		// For internal use only
		return Command::invalid;
	}
	if (str == "set"){
		return Command::set;
	}
	if (str == "math"){
		return Command::math;
	}
	if (str == "if"){
		return Command::gate;
	}
	if (str == "else"){
		return Command::gateOther;
	}
	if (str == "cmpr"){
		return Command::compare;
	}
	if (str == "lcmpr"){
		return Command::longCompare;
	}
	if (str == "bit"){
		return Command::bitwise;
	}
	if (str == "init"){
		return Command::initilize;
	}
	if (str == "bind"){
		return Command::bind;
	}
	if (str == "exec"){
		return Command::execute;
	}
	if (str == "break"){
		return Command::blockExit;
	}
	if (str == "continue"){
		return Command::blockRepeat;
	}
	if (str == "{"){
		return Command::blockOpen;
	}
	if (str == "}"){
		return Command::blockClose;
	}
	if (str == "ss"){
		return Command::standardStream;
	}
	if (str == "mode"){
		return Command::mode;
	}
	if (str == "malloc"){
		return Command::memalloc;
	}
	if (str == "func"){
		return Command::function;
	}
	if (str == "rtrn"){
		return Command::rtrn;
	}
	
	return Command::invalid;
};
