#pragma once

#include "../interprete/function.hpp"
#include "./register.hpp"
#include "./bytecode.hpp"
#include "./memory.hpp"
#include "./thread.hpp"


class Instance{
	public:
		Thread::Worker* owner;

		Instance(Function *reference, Instance *parent = nullptr, Handle* returnValue = nullptr, Order* returnPos = nullptr);
		void Process(size_t position);

		std::mutex lckSessions;
		size_t sessions;

	private:
		Instance *caller;
		Handle *local;
		std::vector<Instance*> child;

		Handle* rtrnVal;
		Order* rtrnPos;

		bool returned;

		Function *instructions;

		Register reg[RegisterCount];

		void AttemptDestruction();
		void MarkChildAsDead(Instance* ptr);


		void CmdMath(
			Interpreter::OpperandType   type1,
			uint64                      data1,
			Interpreter::MathOpperation opperator,
			Interpreter::OpperandType   type2,
			uint64                      data2

		);
		void CmdSet(
			Interpreter::OpperandType type1,
			uint64                    data1,
			bool                      isCustom,
			Interpreter::OpperandType type2,
			uint64                    data2
		);
		void CmdReturn();
};


#include "./instance.cpp"
