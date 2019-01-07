#include "./instance.hpp"


namespace EventLoop{
	void Schedule::Issue(Task task){
		Instance* ptr = reinterpret_cast<Instance*>(task.reference);
		ptr->lckSessions.lock();
		ptr->sessions++;
		ptr->lckSessions.unlock();

		// Prevent other threads from altering while this task is active
		std::lock_guard<std::mutex> lck( this->activity );

		// Add the task to the queue
		this->queue.push_back(task);

		return;
	};
}


Instance::Instance(Function *ref, Instance *prnt, Handle* returnValue, Order* returnPosition){
	this->rtrnPos      = returnPosition;
	this->rtrnVal      = returnValue;
	this->caller       = prnt;
	this->instructions = ref;
	this->returned     = false;

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
			case Command::mode:
				this->reg[ptr->get(0)].setMode( static_cast<RegisterMode>(ptr->get(1)) );
				break;
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
			case Command::set:
				this->CmdSet(
					static_cast<Interpreter::OpperandType>( ptr->get(0) ),
					ptr->get(1),
					(ptr->get(2) == 1),
					static_cast<Interpreter::OpperandType>( ptr->get(3) ),
					ptr->get(4)
				);
				break;
			case Command::math:
				this->CmdMath(
					static_cast<Interpreter::OpperandType>( ptr->get(0) ),
					ptr->get(1),
					static_cast<Interpreter::MathOpperation>( ptr->get(2) ),
					static_cast<Interpreter::OpperandType>( ptr->get(3) ),
					ptr->get(4)
				);
				break;
			case Command::rtrn:
				this->CmdReturn();
				break;
		}

		if (increment == true){
			ptr = code->next(ptr);
		}
	}

	this->lckSessions.lock();
	this->sessions--;
	this->lckSessions.unlock();


	this->AttemptDestruction();
	return;
};


void Instance::CmdMath(Interpreter::OpperandType type1, uint64 data1, Interpreter::MathOpperation opperator, Interpreter::OpperandType type2, uint64 data2){
	if (type1 == Interpreter::OpperandType::RegisterAddress){
		char* ptr = reinterpret_cast<char*>(this->reg[data1].pointer);

		if (type2 == Interpreter::OpperandType::Uint){
			ptr += data2;
		}else{
			ptr += static_cast<int64>(data2);
		}

		this->reg[data1].pointer = reinterpret_cast<Handle*>(ptr);
		return;
	}else{
		switch(type2){
			case Interpreter::OpperandType::Unknown:
				break;
			case Interpreter::OpperandType::RegisterValue:
				if (this->reg[data2].isInt){
					if (this->reg[data2].isSigned){
						switch (opperator){
							case Interpreter::MathOpperation::addition:
								this->reg[data1] += this->reg[data2].toInt64();
								break;
							case Interpreter::MathOpperation::subtract:
								this->reg[data1] -= this->reg[data2].toInt64();
								break;
							case Interpreter::MathOpperation::divide:
								this->reg[data1] /= this->reg[data2].toInt64();
								break;
							case Interpreter::MathOpperation::multiply:
								this->reg[data1] *= this->reg[data2].toInt64();
								break;
							case Interpreter::MathOpperation::modulus:
								this->reg[data1] %= this->reg[data2].toInt64();
								break;
						}

						this->reg[data1] += this->reg[data2].toInt64();
					}else{
						switch (opperator){
							case Interpreter::MathOpperation::addition:
								this->reg[data1] += this->reg[data2].toUint64();
								break;
							case Interpreter::MathOpperation::subtract:
								this->reg[data1] -= this->reg[data2].toUint64();
								break;
							case Interpreter::MathOpperation::divide:
								this->reg[data1] /= this->reg[data2].toUint64();
								break;
							case Interpreter::MathOpperation::multiply:
								this->reg[data1] *= this->reg[data2].toUint64();
								break;
							case Interpreter::MathOpperation::modulus:
								this->reg[data1] %= this->reg[data2].toUint64();
								break;
						}
					}
				}else{
					switch (opperator){
							case Interpreter::MathOpperation::addition:
								this->reg[data1] += this->reg[data2].toFloat64();
								break;
							case Interpreter::MathOpperation::subtract:
								this->reg[data1] -= this->reg[data2].toFloat64();
								break;
							case Interpreter::MathOpperation::divide:
								this->reg[data1] /= this->reg[data2].toFloat64();
								break;
							case Interpreter::MathOpperation::multiply:
								this->reg[data1] *= this->reg[data2].toFloat64();
								break;
							case Interpreter::MathOpperation::modulus:
								this->reg[data1] %= this->reg[data2].toFloat64();
								break;
						}
				}

				return;
			case Interpreter::OpperandType::RegisterAddress:
				switch (opperator){
					case Interpreter::MathOpperation::addition:
						this->reg[data1] += reinterpret_cast<uint64>(this->reg[data2].pointer);
						return;
					case Interpreter::MathOpperation::subtract:
						this->reg[data1] -= reinterpret_cast<uint64>(this->reg[data2].pointer);
						return;
					case Interpreter::MathOpperation::multiply:
						this->reg[data1] *= reinterpret_cast<uint64>(this->reg[data2].pointer);
						return;
					case Interpreter::MathOpperation::divide:
						this->reg[data1] /= reinterpret_cast<uint64>(this->reg[data2].pointer);
						return;
					case Interpreter::MathOpperation::modulus:
						this->reg[data1] %= reinterpret_cast<uint64>(this->reg[data2].pointer);
						return;
				}
				return;
			case Interpreter::OpperandType::Int:
				switch (opperator){
					case Interpreter::MathOpperation::addition:
						this->reg[data1] += static_cast<int64>(data2);
						return;
					case Interpreter::MathOpperation::subtract:
						this->reg[data1] -= static_cast<int64>(data2);
						return;
					case Interpreter::MathOpperation::multiply:
						this->reg[data1] *= static_cast<int64>(data2);
						return;
					case Interpreter::MathOpperation::divide:
						this->reg[data1] /= static_cast<int64>(data2);
						return;
					case Interpreter::MathOpperation::modulus:
						this->reg[data1] %= static_cast<int64>(data2);
						return;
				}
				return;
			case Interpreter::OpperandType::Uint:
				// Overflow to hex
			case Interpreter::OpperandType::Bytes:
				switch (opperator){
					case Interpreter::MathOpperation::addition:
						this->reg[data1] += data2;
						return;
					case Interpreter::MathOpperation::subtract:
						this->reg[data1] -= data2;
						return;
					case Interpreter::MathOpperation::multiply:
						this->reg[data1] *= data2;
						return;
					case Interpreter::MathOpperation::divide:
						this->reg[data1] /= data2;
						return;
					case Interpreter::MathOpperation::modulus:
						this->reg[data1] %= data2;
						return;
				}
				return;
			case Interpreter::OpperandType::Float:
				switch (opperator){
					case Interpreter::MathOpperation::addition:
						this->reg[data1] += static_cast<float64>(data2);
						return;
					case Interpreter::MathOpperation::subtract:
						this->reg[data1] -= static_cast<float64>(data2);
						return;
					case Interpreter::MathOpperation::multiply:
						this->reg[data1] *= static_cast<float64>(data2);
						return;
					case Interpreter::MathOpperation::divide:
						this->reg[data1] /= static_cast<float64>(data2);
						return;
					case Interpreter::MathOpperation::modulus:
						this->reg[data1] %= static_cast<float64>(data2);
				}
				return;
		}
	}

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
void Instance::CmdReturn(){
	this->returned = true;
	this->rtrnVal = nullptr;
};


void Instance::AttemptDestruction(){
	// There is still code waiting to be executed in this instance
	if (this->sessions > 0){
		return;
	}


	// Check if any children remain
	size_t size = this->child.size();
	size_t remain = false;
	for (size_t i=0; i<size; i++){
		if (this->child[i] != nullptr){
			return;
		}
	}


	// If there are no children, double check there are no waiting tasks
	//   Because if there are no children, then nothing else can issue a task for this instance
	this->lckSessions.lock();
	if (this->sessions > 0){
		this->lckSessions.unlock();
		return;
	}
	this->lckSessions.unlock();


	if (this->caller != nullptr){
		this->caller->MarkChildAsDead(this);
	}
	delete this;
};

void Instance::MarkChildAsDead(Instance *ptr){
	size_t size = this->child.size();
	for (size_t i=0; i<size; i++){
		if (this->child[i] == ptr){
			this->child[i] = nullptr;
			break;
		}
	}

	this->AttemptDestruction();
}
