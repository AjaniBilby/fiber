#include "./function.hpp"

Function::Function(std::string name, std::vector<RawAction> tokens){
	this->valid = true;
	this->name = name;
	
	Interpreter::Action temp;
	
	unsigned long size = tokens.size();
	for (unsigned long i=0; i<size; i++){
		temp = Interpreter::Convert(tokens[i]);
		
		if (temp.cmd == Command::invalid){
			this->valid = false;
		}
		
		this->code.append(temp);
	}
	this->code.simplifyJumps();
};
