#pragma once

#include "./../execution/bytecode.hpp"

#include <string>
#include <vector>
#include <stack>

struct FunctionReference{
	void* ptr;
	size_t relDepth;
};

struct __BlockStackElement__{
	size_t start;
	size_t end;
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

		void SimplifyBehaviour();
		void SimplifyBehaviour_initilize();
		void SimplifyBehaviour_if();
		void SimplifyBehaviour_loop();
};


#include "./../interprete/interpreter.hpp"
#include "./function.cpp"
