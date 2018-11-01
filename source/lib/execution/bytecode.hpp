#pragma once

#include "./../command.hpp"

#include <string>
#include <vector>


struct BytecodeCommand{
	Command cmd;           // 2bytes
	unsigned short params; // 2bytes
	unsigned int   lineNo; // 4bytes
};

union BytecodeElement{
	unsigned long          value; // 8bytes
	BytecodeCommand cmd;          // 8Bytes
};

struct BytecodeReference{
	unsigned long index;
	unsigned long _trueIndex;
};


class Bytecode{
	public:
		std::size_t      find(std::size_t commandNum); // Find the true index of the command
		BytecodeElement* at(std::size_t index);        // Get a reference to the element at the 'true' index
		
		std::size_t next(std::size_t idx);             // Get the index of the next command
		
		// Add a new command with parameters to he end of the code
		void append(Command cmd, std::vector<unsigned long> params);
		
		// Change jumps from being relative to commands to being exact 'true' indexes
		void simplifyJumps();
	private:
		std::vector<BytecodeElement> data;
};


std::string ToString(BytecodeElement* code);


#include "./bytecode.cpp"
