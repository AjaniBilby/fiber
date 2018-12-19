#include "./instance.hpp"


Instance::Instance(Function *ref, Instance *prnt = nullptr){
	this->instructions = ref;
	this->local = Memory::Allocate(ref->domain);
	this->parent = prnt;
};



void Instance::Process(size_t pos = 0){
	Bytecode* code = &this->instructions->code;
	auto ptr = code->next();
	bool increment = true;

	std::cout << "Executing " << ptr << std::endl;

	while (ptr != nullptr){
		std::cout << increment << " ";
		increment = true;

		std::cout << ToString(ptr) << std::endl;

		switch(ptr->cmd){
			case Command::jump:
				ptr = reinterpret_cast<Order*>( ptr->get(0) );
				increment = false;
				break;
			case Command::stop:
				// Trigger the while loop to stop on next itteration
				increment = false;
				ptr = nullptr;
				break;
		}

		if (increment == true){
			ptr = code->next(ptr);
		}
	}

	return;
};


void Instance::CmdMath(){
	// DON'T FORGET THAT POINTER ADDITION IS WEIRD
};
