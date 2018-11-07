#pragma once

#include "./../execution/bytecode.hpp"

#include <string>
#include <vector>

class Function{
	public:
		Function* parent;
		std::string name;

		Bytecode code;
		std::vector<Function> child;

		bool valid;
		size_t domain;

		Function(std::string name, std::vector<RawAction> tokens, size_t domainSize, Function* owner=nullptr);
};


#include "./../interprete/interpreter.hpp"
#include "./function.cpp"
