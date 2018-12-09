#pragma once

#include "../interprete/function.hpp"
#include "./register.hpp"
#include "./bytecode.hpp"
#include "./memory.hpp"
#include "./thread.hpp"


class Instance{
	public:
		Thread::Worker* owner;
		Instance *parent;
		void* local;

		Function *instructions;

		Register reg[RegisterCount];

		Instance(Function *reference, Instance *parent);
		void Process(size_t position);
};


#include "./instance.cpp"
