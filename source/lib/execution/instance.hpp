#pragma once

#include "./register.hpp"
#include "./memory.hpp"
#include "./thread.hpp"


class Instance{
	public:
		Thread::Worker* owner;

		Register reg[RegisterCount];

		void Process(size_t position);
};


#include "./instance.cpp"
