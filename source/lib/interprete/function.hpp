#pragma once

#include "./../execution/bytecode.hpp"

#include <string>
#include <vector>

struct __BlockStackElement__{
	size_t start;
	size_t end;
};

class Function{
	public:
		void* parent; // Table*
		std::string name;

		Bytecode code;

		bool valid;
		size_t domain;

		Function(std::string name, std::vector<RawAction> tokens, size_t domainSize, void* owner=nullptr);

		void Finalize();
	private:
		std::vector<Action> actions;

		void SimplifyBehaviour();
		void SimplifyBehaviour_initilize();
		void SimplifyBehaviour_if();
		void SimplifyBehaviour_loop();
};


#include "./../interprete/interpreter.hpp"
#include "./table.hpp"
#include "./function.cpp"
