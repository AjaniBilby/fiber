#pragma once

#include "./../execution/register.hpp"
#include "./../typebase.hpp"
#include "./../command.hpp"
#include "./tokenize.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>


namespace Interpreter{
	typedef std::vector<std::string> Parameters;

	enum class OpperandType{
		Unknown,
		RegisterValue,
		RegisterAddress,
		Float,
		Int,
		Uint,
		Bytes
	};

	class Opperand{
		public:
			bool valid;
			OpperandType type;
			Handle data;

			Opperand();
			Opperand(std::string);

			std::string typeToString();
	};
	
	
	struct Action{
		Command cmd;
		uint64 line;
		std::vector<uint64> param;
	};
};


#include "./interpreter.cpp"
