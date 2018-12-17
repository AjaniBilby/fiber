#pragma once

#include "./../execution/bytecode.hpp"

#include <string>
#include <vector>
struct FunctionReference{
	void* ptr;
	size_t relDepth;
};

class Function{
	public:
		Function* parent;
		std::string name;

		Bytecode code;
		std::vector<Function> child;

		bool valid;
		size_t domain;

		Function(std::string name, std::vector<RawAction> tokens, size_t domainSize, Function* owner=nullptr);

		// Find a function with the supplied name
		FunctionReference find(std::string str);

		void finalize();
	private:
		std::vector<Action> actions;
};


#include "./../interprete/interpreter.hpp"
#include "./function.cpp"
