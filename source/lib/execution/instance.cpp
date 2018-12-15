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

	while (ptr != nullptr){
		increment = true;

		if (increment == true){
			ptr = code->next(ptr);
		}
	}

	return;
};


void Instance::CmdMath(){
	// DON'T FORGET THAT POINTER ADDITION IS WEIRD
};
