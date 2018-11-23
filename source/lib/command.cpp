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
		case Command::bitwise:
			return "bitwise";
		case Command::initilize:
			return "init";
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
			return "stop";
		case Command::jump:
			// For internal use only
			return "Err(CMD:jump)";
	}

	return "Err";
};

Command CommandFrom(std::string str){
	if        (str == "stop"){
		return Command::stop;
	} else if (str == "blank"){
		// For internal use only
		return Commmand::invalid;
	} else if (str == "jump"){
		// For internal use only
		return Command::invalid;
	} else if (str == "set"){
		return Command::set;
	} else if (str == "math"){
		return Command::math;
	} else if (str == "if"){
		return Command::gate;
	} else if (str == "else"){
		return Command::gateOther;
	} else if (str == "cmpr"){
		return Command::compare;
	} else if (str == "bit"){
		return Command::bitwise;
	} else if (str == "init"){
		return Command::initilize;
	} else if (str == "break"){
		return Command::blockExit;
	} else if (str == "continue"){
		return Command::blockRepeat;
	} else if (str == "{"){
		return Command::blockOpen;
	} else if (str == "}"){
		return Command::blockClose;
	} else if (str == "ss"){
		return Command::standardStream;
	} else if (str == "mode"){
		return Command::mode;
	} else if (str == "malloc"){
		return Command::memalloc;
	} else if (str == "func"){
		return Command::function;
	} else if (str == "rtrn"){
		return Command::rtrn;
	}

	return Command::invalid;
};
