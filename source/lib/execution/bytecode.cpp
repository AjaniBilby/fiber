#include "./bytecode.hpp"

std::size_t Bytecode::find(std::size_t commandNum){	
	unsigned long size = this->data.size();
	unsigned long i=0;
	
	// Jump from element header to element header
	while( i<size ){
		i += this->data[i].cmd.params;
		commandNum--;
		
		// At the correct index
		if (commandNum == 0){
			return i;
		}
	}
	
	return size;
};
BytecodeElement* Bytecode::at(std::size_t index){
	// Check the index is valid
	if (index > this->data.size()){
		return nullptr;
	}
	
	return &this->data[index];
};

std::size_t Bytecode::next(std::size_t index){
	return index + this->data[index].cmd.params;
};

void Bytecode::append(Command cmd, std::vector<unsigned long> params){
	unsigned long    i = this->data.size();
	unsigned long size = params.size();
	
	// Create space for the element header + params
	this->data.resize( this->data.size()+size+1 );
	
	// Set the element header
	this->data[i].cmd = { reinterpret_cast<Command>(cmd), size };
	// Set the individual parameters
	for (unsigned long j = 0; j<size; j++){
		this->data[j+1].value = params[j];
	}
};

void Bytecode::simplifyJumps(){
	unsigned long size = this->data.size();
	unsigned long    i = 0;
	unsigned long  num = 0;
	
	while (i < size){
		if (this->data[i].cmd.cmd == Command::jump){
			// Change the jump commands amount to an exact element index
			//                                       Reinterprete the param to be signed to allow backwards jumps
			this->data[i+1].value = this->find(num + static_cast<long>(this->data[i+1].value))._trueIndex;
		}
		
		i += this->data[i].cmd.params;
		num++;
	}
};




std::string ToString(BytecodeElement* code){
	unsigned long size = code->cmd.params;
	
	// Get the CommandID as a string
	std::string str = ToString(code->cmd.cmd);
	
	while (size > 0){
		code += sizeof(BytecodeElement); // Shift to the next parameter
		
		str += " " + std::to_string(code->value);
		
		size--;
	}
	
	return str;
};
