#pragma once

#include "./../execution/bytecode.hpp"
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

	enum class MathOpperation{
		addition,
		subtract,
		divide,
		multiply,
		modulus
	};

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
			bool          valid;
			OpperandType  type;
			Handle        data;

			Opperand ();
			Opperand (std::string);

			std::string typeToString();
	};


	Action Convert(RawAction act, Function* context);
};


#include "./function.hpp"
#include "./interpreter.cpp"
