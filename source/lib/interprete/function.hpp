#pragma once

#include "./../interprete/tokenize.hpp"
#include "./../execution/bytecode.hpp"

#include <string>
#include <vector>


class Function{
	public:
		std::string name;
	
		Bytecode code;
		std::vector<Function> child;
		
		bool valid;
		
		Function(std::string name, std::vector<RawAction> tokens);
};


#include "./function.cpp"
