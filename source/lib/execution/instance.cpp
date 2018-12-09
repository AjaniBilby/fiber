#include "./instance.hpp"


Instance::Instance(Function *ref, Instance *prnt = nullptr){
	this->instructions = ref;
	this->local = Memory::Allocate(ref->domain);
	this->parent = prnt;
};



void Instance::Process(size_t pos = 0){
	std::string str;
	str = "Executing " + std::to_string(pos) + "\n";
	std::cout << str;

	Bytecode* code = &this->instructions->code;
	auto ptr = code->next();
	bool increment = true;

	// while (ptr != nullptr){
	// 	increment = true;

	// 	std::cout << ToString(ptr) << std::endl;
	// 	std::cout << "  " << ptr << std::endl;

	// 	if (increment == true){
	// 		ptr = code->next(ptr);
	// 	}
	// }

	return;
};
