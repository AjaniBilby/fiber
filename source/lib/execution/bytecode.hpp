#pragma once

#include "./../typebase.hpp"
#include "./../command.hpp"

#include <string>
#include <vector>
#include <memory>


struct Action{
	Command cmd;
	size_t line;
	std::vector<uint64> param;
};

class Order{
	public:
		Command cmd;
		uint32 line;
		uint8 params;

		Order* next;

		uint64  get(size_t i=0);
		uint64* ref(size_t i=0);
};


class Bytecode{
	public:
		~Bytecode();

		// Add a new command to the chain
		void append(Action act);

		// Get the next sequential order
		Order* next(Order* current = nullptr);
		Order* getLast();

		// Get a specific order by sequential index
		Order* at(size_t i);

		bool simplify();

	private:
		Order* first = nullptr;
		Order* last = nullptr;

		uint64* OrderParamRef(Order* ptr, size_t i);

		// Convert jumps to exact address references
		bool simplifyJumps();
		// Remove blank commands
		bool simplifyRemoveBlanks();
};




std::string ToString(Action code);


#include "./bytecode.cpp"
