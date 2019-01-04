#include "./instance.hpp"


Instance::Instance(Function *ref, Instance *prnt, Handle* returnValue, Order* returnPosition){
	this->rtrnPos      = returnPosition;
	this->rtrnVal      = returnValue;
	this->caller       = prnt;
	this->instructions = ref;

	if (ref->domain == 0){
		this->local = reinterpret_cast<Handle*>( Memory::Allocate(ref->domain) );
	}else{
		this->local = nullptr;
	}
};



void Instance::Process(size_t pos = 0){
	Bytecode* code = &this->instructions->code;
	auto ptr = code->next();
	bool increment = true;

	std::cout << "Executing " << ptr << std::endl;

	while (ptr != nullptr){
		increment = true;

		std::cout << ToString(ptr) << std::endl;

		switch(ptr->cmd){
			case Command::jump:
				ptr = reinterpret_cast<Order*>( ptr->get(0) );
				increment = false;
				break;
			case Command::gate:
				if (this->reg[ ptr->get(0) ].IsTrue() == true){ // Go over the jump to else/end
					ptr = code->next(ptr);
					ptr = code->next(ptr);
					increment = false;
				}else{                                          // Goto the jump to else/end
					ptr = code->next(ptr);
					increment = false;
				}

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
void Instance::CmdSet(Interpreter::OpperandType type1, uint64 data1, bool isCustom, Interpreter::OpperandType type2, uint64 data2){
	// See # docs/language/commands/set.md for valid combinations

	if (type1 == Interpreter::OpperandType::RegisterAddress){

		if       (type2 == Interpreter::OpperandType::RegisterValue){
			this->reg[data1].set( &this->reg[data2] );
		}else if (type2 == Interpreter::OpperandType::RegisterAddress){
			this->reg[data1].set(reinterpret_cast<uint64>( this->reg[data2].pointer ));
		}else if (type2 == Interpreter::OpperandType::Float){
			this->reg[data1].set(static_cast<float64>( data2 ));
		}else if (type2 == Interpreter::OpperandType::Int){
			this->reg[data1].set(static_cast<int64>( data2 ));
		}else if (type2 == Interpreter::OpperandType::Uint || type2 == Interpreter::OpperandType::Bytes){
			this->reg[data1].set(static_cast<uint64>( data2 ));
		}

	}else{
		if (isCustom == true){
			if       (data2 == 1){ // local
				this->reg[data1].pointer = this->local;
			}else if (data2 == 2){ // parse
				this->reg[data1].pointer = this->rtrnVal;
			}
		} else {
			if       (type2 == Interpreter::OpperandType::RegisterValue){
				this->reg[data1].pointer = reinterpret_cast<Handle*>( this->reg[data2].toUint64() );
			}else if (type2 == Interpreter::OpperandType::RegisterAddress){
				this->reg[data1].pointer = this->reg[data2].pointer;
			}
		}
	}
};
void Instance::CmdReturn();
