#include "instance.hpp"




Instance::Instance (Function *func, Instance *caller, Thread::Pool *pool, Instance *parent){
	this->ref         = func;
	this->parent      = parent;
	this->caller      = caller;
	this->pool        = pool;
	this->localMemory = nullptr;
	this->connected   = true;

	this->child.reserve(0);
};
void Instance::Execute (unsigned long cursor, void* data, unsigned long size){
	// Don't execute if this instance has destoyed
	if (this->HasDestoryed() == true){
		std::cout << "Already destoyed, shouldn't be executing" << std::endl;
		return;
	}

	// Itterative data
	int length = this->ref->code.size();
	Action *act;

	// Working data
	unsigned int temp = 0;
	bool stop = false;

	// Debug data
	std::string msg;

	msg = "Execution: " + std::to_string(this->workerID);
	std::cout << msg << std::endl;

	// Itterativly interpret
	while (cursor < length){
		act = &this->ref->code[cursor];

		msg =  " " + std::to_string(this->workerID) + "|" + std::to_string(act->line);
		msg += " > Command[" + std::to_string(act->command) + "] " + std::to_string(cursor) + " " + std::to_string(length);
		std::cout << msg << std::endl;


		// Siphon specific behaviour
		switch (act->command){
			case Commands::math:
				this->CmdMath(act);
				break;
			case Commands::compare:
				this->CmdComp(act);
				break;
			case Commands::set:
				this->CmdSet(act);
				break;
			case Commands::push:
				this->CmdPush(act);
				break;
			case Commands::pull:
				this->CmdPull(act);
				break;
			case Commands::mode:
				this->CmdMode(act);
				break;
			case Commands::Clone:
				this->CmdClone(act);
				break;
			case Commands::translate:
				this->CmdTranslate(act);
				break;
			case Commands::move:
				this->CmdMove(act);
				break;
			case Commands::instance:
				// Allow instance to direct execution flow to jump over yeild&return code
				temp = this->CmdInstance(act, cursor);
				if (temp != 0){
					// Set the cursor position for the next itteration
					cursor = temp;
					continue;
				}

				break;
			case Commands::local:
				this->CmdLocal(act);
				break;
			case Commands::IF:
				if (this->handle[ act->param[0] ].value.uint8 == 0){
					cursor = act->param[1];
				}
				break;
			case Commands::GOTO:
				cursor = act->param[0];
				break;
			case Commands::data:
				this->handle[ act->param[0] ].value.address = data;
				if (act->param[1] == 1){
					this->handle[ act->param[2] ].value.uint64 = size;
				}
				break;
			case Commands::memory:
				this->CmdMem(act);
				break;
			case Commands::standardStream:
				this->CmdSS(act);
				break;

			case Commands::longCompare:
				this->CmdLComp(act);
				break;
			case Commands::bitwise:
				this->CmdBit(act);
				break;

			case Commands::Loop:
				break;
			case Commands::blank:
				break;
			case Commands::stop:
				return;
			case Commands::Yeild:
				this->CmdYeild(act);
				break;
			case Commands::Return:
				this->CmdReturn(act);
				stop = true;
				break;
			case Commands::stringify:
				this->CmdStringify(act);
				break;
			case Commands::write:
				this->CmdWrite(act);
				break;
			case Commands::invalid:
				std::cerr << "Warn: Unexpected invalid command" << std::endl;
				std::cerr << "  line: " << act->line << std::endl;
				break;
			default:
				msg =  "Warn: Unknown command " + std::to_string(act->command) + "\n";
				msg += "  line: " + std::to_string(act->line);
				std::cerr << msg << std::endl;
				break;
		}

		if (stop){
			break;
		}

		cursor += 1;
		continue;
	}

	// Unallocate parsed data
	//   Avoid memory leaks
	if (size > 0){
		Memory::UnAllocate(data);
	}

	msg = "Executed: " + std::to_string(this->workerID);
	std::cout << msg << std::endl;
};



bool Instance::IsChild(Instance *ptr){
	unsigned long length = this->child.size();

	// Search for the child
	for (int i=0; i<length; i++){
		// Ignore invalid elements
		if (this->child[i] == nullptr){
			continue;
		}

		// Return on match
		if (this->child[i] == ptr){
			return true;
		}

		// Allow for children by proxy
		if (this->child[i]->IsChild(ptr) == true){
			return true;
		}
	}

	// Failed to find a match
	return false;
}





bool Instance::HasDestoryed(){
	this->sensitive.lock();
	bool res = !this->connected;
	this->sensitive.unlock();

	return res;
}

void Instance::Destory(){
	this->sensitive.lock();

	// The instance has already decayed
	if (this->connected == false){
		this->sensitive.unlock();
		return;
	}
	this->connected = false;

	Thread::Job task;
	task.conclude = true;
	task.cursor = 0;
	unsigned long length = this->child.size();
	for (unsigned long i=0; i<length; i++){
		this->child[i]->sensitive.lock();

		// Tell the instance that it no longer has a parent
		this->child[i]->connected = false;

		// If the child may already be actie
		if (this->child[i]->assigned){
			// Remove all other actions for the child
			this->pool->Recall(this->child[i]);

			// Tell the instance to self destory
			//  It maybe mid exeuction
			this->pool->Dispatch(task, true, this->child[i]->workerID);

		}

		this->child[i]->sensitive.unlock();
	}


	// Remove any tasks using this instance
	//  Note: they may of been generated during this function processing
	this->pool->Recall(this);

	// If this instance has local memory
	//  free it
	if (this->localMemory != nullptr){
		Memory::UnAllocate(this->localMemory);
	}

	// Ensure that the parent removes the child
	if (this->parent != nullptr){
		this->parent->RemoveChild(this);
	}

	this->sensitive.unlock();
	delete this;
}

void Instance::RemoveChild(Instance *ptr){
	unsigned long length = this->child.size();

	for (unsigned long i=0; i<length; i++){
		if (this->child[i] == ptr){
			this->child[i] = nullptr;
			break;
		}
	}
}





void* Instance::GetLocalSpace(){

	// If no local memory has been generated yet
	//   Allocate it
	if (this->localMemory == 0){
		this->sensitive.lock();

		// Check it hasn't changed during blocking operation
		if (this->localMemory == 0){
			this->localMemory = Memory::Allocate(this->ref->localSize);
		}

		this->sensitive.unlock();
	}

	return this->localMemory;
}




/*------------------------------------------
		Commands
------------------------------------------*/
void Instance::CmdSet       (Action *act){
	// Set type (direct/referencial)
	if (act->param[1] == 1){
		// Copy register values across
		this->handle[ act->param[0] ].write(  this->handle[ act->param[2] ].read()  );
	}else{
		// Set the literal value of a register
		this->handle[ act->param[0] ].write( act->param[2] );
	}
};
void Instance::CmdSS        (Action *act){
	std::vector<char> bytes;
	unsigned long count;
	std::string str;

	// Ensure text pointer is perceivably valid
	if (this->handle[ act->param[1] ].mode != RegisterMode::uint64){
		std::cerr << "Warn: Attempting to use standard stream with an address register of non-uint64 mode" << std::endl;
		std::cerr << "  Mode: "<<this->handle[ act->param[1] ].mode                                        << std::endl;
		std::cerr << "  Line: "<<act->line                                                                 << std::endl;
	}

	if (act->param[0] == 0){
		// Standard-in Behvaiour

		// Ensure text length is perceivably valid
		if (this->handle[ act->param[2] ].mode != RegisterMode::uint64){
			std::cerr << "Warn: Standard Stream length result register should be in uint64 mode to prevent overloading" << std::endl;
			std::cerr << "  Mode: " << this->handle[ act->param[2] ].mode                                               << std::endl;
			std::cerr << "  Line: " << act->line                                                                        <<std::endl;
		}

		std::cin >> str;
		count = sizeof(str);

		// Create a safe space for the input stream data
		//   (Won't get destroyed after function call)
		// Store the information about the space in the
		// two specified registers
		this->handle[ act->param[1] ].value.address = Memory::Allocate ( count );
		this->handle[ act->param[2] ].write(count);

		// Move the data to the space space
		Memory::Duplicate(this->handle[ act->param[1] ].value.address, &str, count );

	}else{
		// All standard output behaviour

		// Interpret string pointer + length into C++ types
		char *ptr = reinterpret_cast<char* >(this->handle[ act->param[1] ].value.address);
		count = this->handle[ act->param[2] ].read();

		// Read the selected chunk of memory into a char-array for string conversion
		bytes.resize(count);
		for (unsigned long i=0; i<count; i++){
			bytes[i] = *(ptr + i);
		}

		// Convert char array into a string
		std::string str(bytes.begin(), bytes.end());

		// Pipe the string into the correct output channel
		switch (act->param[0]){
			case 1:
				std::cout << str;
				break;
			case 2:
				std::cerr << str;
				break;
			case 3:
				std::clog << str;
				break;
		}
	}
};
void Instance::CmdMem       (Action *act){
	// Make sure the pointer register is of valid type
	if (this->handle[ act->param[1] ].mode != RegisterMode::uint64){
		std::cerr << "Warn: Attempting to (un)allocate memory with an address register of non-uint64 mode" << std::endl;
		std::cerr << "  Mode: "<<this->handle[ act->param[1] ].mode<<std::endl;
		std::cerr << "  Line: "<<act->line<<std::endl;
	}

	// Switch between (un)allocate behaviour
	if (act->param[0] == 0){
		this->handle[ act->param[1] ].value.address = Memory::Allocate ( this->handle[ act->param[2] ].read() );
	}else{
		Memory::UnAllocate ( this->handle[ act->param[1] ].value.address );
	}
};
void Instance::CmdPush      (Action *act){
	// Setup write location
	Handle *director = reinterpret_cast<Handle*>(this->handle[ act->param[1] ].value.address);

	// Switch between number of bytes
	switch (this->handle[ act->param[0] ].mode){
		// Write 1byte
		case RegisterMode::int8:
		case RegisterMode::uint8:
			director->uint8 = this->handle[ act->param[0] ].value.uint8;
			break;

		// Write 2bytes
		case RegisterMode::int16:
		case RegisterMode::uint16:
			director->uint16 = this->handle[ act->param[0] ].value.uint16;
			break;

		// Write 4bytes
		case RegisterMode::int32:
		case RegisterMode::uint32:
		case RegisterMode::float32:
			director->uint32 = this->handle[ act->param[0] ].value.uint32;
			break;

		// Write 8bytes
		case RegisterMode::int64:
		case RegisterMode::uint64:
		case RegisterMode::float64:
			director->uint64 = this->handle[ act->param[0] ].value.uint64;
			break;
	}
};
void Instance::CmdPull      (Action *act){
	// Setup read location
	Handle *director = reinterpret_cast<Handle*>(this->handle[ act->param[1] ].value.address);

	// Switch between number of bytes
	switch (this->handle[ act->param[0] ].mode){
		// Read 1byte
		case RegisterMode::int8:
		case RegisterMode::uint8:
			this->handle[ act->param[0] ].value.uint8 = director->uint8;

		// Read 2bytes
		case RegisterMode::int16:
		case RegisterMode::uint16:
			this->handle[ act->param[0] ].value.uint16 = director->uint16;

		// Read 4bytes
		case RegisterMode::int32:
		case RegisterMode::uint32:
		case RegisterMode::float32:
			this->handle[ act->param[0] ].value.uint32 = director->uint32;

		// Read 8bytes
		case RegisterMode::int64:
		case RegisterMode::uint64:
		case RegisterMode::float64:
			this->handle[ act->param[0] ].value.uint64 = director->uint64;
	}
};
void Instance::CmdMode      (Action *act){
	// Set the mode of the register without translating any data
	this->handle[ act->param[0] ].mode = static_cast<RegisterMode>(act->param[1]);
};
void Instance::CmdTranslate (Action *act){
	// Change the type of the register while keeping the same information
	this->handle[ act->param[0] ].Translate( static_cast<RegisterMode>(act->param[1]) );
};
void Instance::CmdMath      (Action *act){
	Register A;
	Register B;
	Register *C;
	RegisterMode goal;

	// Value A
	if (act->param[0] == 1){
		A.mode = this->handle[ act->param[1] ].mode;
		A.write( this->handle[ act->param[1] ].read() );
	}else{
		A.mode = RegisterMode::uint64;
		A.value.uint64 = act->param[1];
	}
	// Value B
	if (act->param[2] == 1){
		B.mode = this->handle[ act->param[3] ].mode;
		B.write( this->handle[ act->param[3] ].read() );
	}else{
		B.mode = RegisterMode::uint64;
		B.value.uint64 = act->param[3];
	}

	// Create an easy reference for result location
	C = &this->handle[ act->param[4] ];
	goal = C->mode; // Ensure the output type is never lost

	// Siphon between different maths operations
	switch ( static_cast<MathOpperation>(act->param[5]) ){
		case MathOpperation::add:
			A.Translate( C->mode );
			B.Translate( C->mode );

			// Give float32/64 the correct behvaiour
			// Treat all ints as uint64 as it will not effect TCI addition behvaiour
			if (C->mode == RegisterMode::float32){
				C->value.float32 = A.value.float32 + B.value.float32;
			}else if (C->mode == RegisterMode::float64){
				C->value.float64 = A.value.float64 + B.value.float64;
			}else{
				auto temp1 = A.read();
				auto temp2 = B.read();
				auto temp3 = temp1 + temp2;
				C->write(temp3);
			}
			break;
		case MathOpperation::subtract:
			A.Translate( C->mode );
			B.Translate( C->mode );

			// Give float32/64 the correct behvaiour
			// Treat all ints as uint64 as it will not effect TCI addition behvaiour
			if (C->mode == RegisterMode::float32){
				C->value.float32 = A.value.float32 + B.value.float32;
			}else if (C->mode == RegisterMode::float64){
				C->value.float64 = A.value.float64 + B.value.float64;
			}else{
				C->write( A.read() - B.read() );
			}
			break;
		case MathOpperation::multiply:
			// Use the correct float precision for the output result
			if (C->mode == RegisterMode::float64){
				A.Translate(RegisterMode::float64);
				B.Translate(RegisterMode::float64);

				C->value.float64 = A.value.float64 * B.value.float64;
			}else if (C->mode == RegisterMode::float32){
				A.Translate(RegisterMode::float32);
				B.Translate(RegisterMode::float32);

				C->value.float32 = A.value.float32 * B.value.float32;
			}else{
				// TCI will mean signature and sizing of integers will not matter
				C->write( A.read() * B.read() );
			}

			break;
		case MathOpperation::divide:
			// Use the correct float precision for the output result
			if (C->mode == RegisterMode::float64){
				A.Translate(RegisterMode::float64);
				B.Translate(RegisterMode::float64);

				C->value.float64 = A.value.float64 / B.value.float64;
			}else if (C->mode == RegisterMode::float32){
				A.Translate(RegisterMode::float32);
				B.Translate(RegisterMode::float32);

				C->value.float32 = A.value.float32 / B.value.float32;
			}else{
				// TCI will mean signature and sizing of integers will not matter
				C->write( A.read() / B.read() );
			}

			break;
		case MathOpperation::modulus:
			// Remember fmod is float64 only
			if (C->mode == RegisterMode::float64 || C->mode == RegisterMode::float32){
				A.Translate(RegisterMode::float64);
				B.Translate(RegisterMode::float64);
				C->mode = RegisterMode::float64;

				C->value.float64 = fmod(A.value.float64, B.value.float64);
				C->Translate(goal);
			}else{
				C->write( A.read() % B.read() );
			}
			break;
		case MathOpperation::exponent:
			// Remember pow is float64 only
			A.Translate(RegisterMode::float64);
			B.Translate(RegisterMode::float64);
			C->mode = RegisterMode::float64;

			C->value.float64 = pow(A.value.float64, B.value.float64);
			C->Translate(goal);
			break;
	}
};
void Instance::CmdClone     (Action *act){
	// Make the target register a mirror of the original
	//   Use pointers to safe on array compute costs
	Register *A = &this->handle[ act->param[0] ];
	Register *B = &this->handle[ act->param[1] ];

	B->mode = A->mode;
	B->value.uint64 = A->value.uint64; // Just copy bytes across
};
void Instance::CmdMove      (Action *act){
	// Check the supplied pointers are of valid type
	if (this->handle[ act->param[0] ].mode != RegisterMode::uint64){
		std::cerr << "Warn: Attempting to move data using a register for the from address not in uint64 mode" << std::endl;
		std::cerr << "  Mode: "<<this->handle[ act->param[0] ].mode                                           << std::endl;
		std::cerr << "  Line: "<<act->line                                                                    << std::endl;
	};
	if (this->handle[ act->param[1] ].mode != RegisterMode::uint64){
		std::cerr << "Warn: Attempting to move data using a register for the to address not in uint64 mode" << std::endl;
		std::cerr << "  Mode: "<<this->handle[ act->param[1] ].mode                                         << std::endl;
		std::cerr << "  Line: "<<act->line                                                                  << std::endl;
	};

	// Duplicate one section of RAM to another
	Memory::Duplicate(
		this->handle[ act->param[1] ].value.address,  // To
		this->handle[ act->param[0] ].value.address,  // From
		this->handle[ act->param[2] ].read()          // Bytes
	);
}
void Instance::CmdComp      (Action *act){
	//   Use pointers to safe on array compute costs
	Register *A = &this->handle[ act->param[0] ];
	Register *B = &this->handle[ act->param[2] ];
	Register *C = &this->handle[ act->param[3] ];

	// Switch between all possible combinations of register type
	// to ensure that the comparason is at the information level instead of the data/byte level
	switch (A->mode){
		case uint8:
			switch ( B->mode ){
				case uint8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint8 == B->value.uint8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint8 >  B->value.uint8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint8 <  B->value.uint8 ? 255 : 0;
							break;
					}

					return;
				case int8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint8 == B->value.int8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint8 >  B->value.int8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint8 <  B->value.int8 ? 255 : 0;
							break;
					}

					return;
				case uint16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint8 == B->value.uint16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint8 >  B->value.uint16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint8 <  B->value.uint16 ? 255 : 0;
							break;
					}

					return;
				case int16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint8 == B->value.int16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint8 >  B->value.int16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint8 <  B->value.int16 ? 255 : 0;
							break;
					}

					return;
				case float32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint8 == B->value.float32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint8 >  B->value.float32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint8 <  B->value.float32 ? 255 : 0;
							break;
					}

					return;
				case uint32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint8 == B->value.uint32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint8 >  B->value.uint32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint8 <  B->value.uint32 ? 255 : 0;
							break;
					}

					return;
				case int32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint8 == B->value.int32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint8 >  B->value.int32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint8 <  B->value.int32 ? 255 : 0;
					}

					return;
				case float64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint8 == B->value.float64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint8 >  B->value.float64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint8 <  B->value.float64 ? 255 : 0;
					}

					return;
				case uint64:

					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint8 == B->value.uint64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint8 >  B->value.uint64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint8 <  B->value.uint64 ? 255 : 0;
							break;
					}

					return;
				case int64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint8 == B->value.int64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint8 >  B->value.int64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint8 <  B->value.int64 ? 255 : 0;
							break;
					}

					return;
			}

			break;
		case int8:
			switch ( B->mode ){
				case uint8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int8 == B->value.uint8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int8 >  B->value.uint8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int8 <  B->value.uint8 ? 255 : 0;
							break;
					}

					return;
				case int8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int8 == B->value.int8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int8 >  B->value.int8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int8 <  B->value.int8 ? 255 : 0;
							break;
					}

					return;
				case uint16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int8 == B->value.uint16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int8 >  B->value.uint16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int8 <  B->value.uint16 ? 255 : 0;
							break;
					}

					return;
				case int16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int8 == B->value.int16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int8 >  B->value.int16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int8 <  B->value.int16 ? 255 : 0;
							break;
					}

					return;
				case float32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int8 == B->value.float32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int8 >  B->value.float32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int8 <  B->value.float32 ? 255 : 0;
							break;
					}

					return;
				case uint32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int8 == B->value.uint32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int8 >  B->value.uint32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int8 <  B->value.uint32 ? 255 : 0;
							break;
					}

					return;
				case int32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int8 == B->value.int32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int8 >  B->value.int32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int8 <  B->value.int32 ? 255 : 0;
							break;
					}

					return;
				case float64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int8 == B->value.float64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int8 >  B->value.float64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int8 <  B->value.float64 ? 255 : 0;
							break;
					}

					return;
				case uint64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int8 == B->value.uint64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int8 >  B->value.uint64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int8 <  B->value.uint64 ? 255 : 0;
							break;
					}

					return;
				case int64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int8 == B->value.int64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int8 >  B->value.int64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int8 <  B->value.int64 ? 255 : 0;
							break;
					}

					return;
			}

			break;
		case uint16:
			switch ( B->mode ){
				case uint8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint16 == B->value.uint8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint16 >  B->value.uint8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint16 <  B->value.uint8 ? 255 : 0;
							break;
					}

					return;
				case int8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint16 == B->value.int8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint16 >  B->value.int8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint16 <  B->value.int8 ? 255 : 0;
							break;
					}

					return;
				case uint16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint16 == B->value.uint16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint16 >  B->value.uint16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint16 <  B->value.uint16 ? 255 : 0;
							break;
					}

					return;
				case int16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint16 == B->value.int16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint16 >  B->value.int16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint16 <  B->value.int16 ? 255 : 0;
					}

					return;
				case float32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint16 == B->value.float32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint16 >  B->value.float32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint16 <  B->value.float32 ? 255 : 0;
							break;
					}

					return;
				case uint32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint16 == B->value.uint32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint16 >  B->value.uint32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint16 <  B->value.uint32 ? 255 : 0;
							break;
					}

					return;
				case int32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint16 == B->value.int32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint16 >  B->value.int32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint16 <  B->value.int32 ? 255 : 0;
							break;
					}

					return;
				case float64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint16 == B->value.float64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint16 >  B->value.float64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint16 <  B->value.float64 ? 255 : 0;
							break;
					}

					return;
				case uint64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint16 == B->value.uint64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint16 >  B->value.uint64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint16 <  B->value.uint64 ? 255 : 0;
							break;
					}

					return;
				case int64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint16 == B->value.int64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint16 >  B->value.int64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint16 <  B->value.int64 ? 255 : 0;
							break;
					}

					return;
			}

			break;
		case int16:
			switch ( B->mode ){
				case uint8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int16 == B->value.uint8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int16 >  B->value.uint8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int16 <  B->value.uint8 ? 255 : 0;
							break;
					}

					return;
				case int8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int16 == B->value.int8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int16 >  B->value.int8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int16 <  B->value.int8 ? 255 : 0;
							break;
					}

					return;
				case uint16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int16 == B->value.uint16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int16 >  B->value.uint16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int16 <  B->value.uint16 ? 255 : 0;
							break;
					}

					return;
				case int16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int16 == B->value.int16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int16 >  B->value.int16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int16 <  B->value.int16 ? 255 : 0;
							break;
					}

					return;
				case float32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int16 == B->value.float32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int16 >  B->value.float32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int16 <  B->value.float32 ? 255 : 0;
							break;
					}

					return;
				case uint32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int16 == B->value.uint32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int16 >  B->value.uint32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int16 <  B->value.uint32 ? 255 : 0;
							break;
					}

					return;
				case int32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int16 == B->value.int32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int16 >  B->value.int32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int16 <  B->value.int32 ? 255 : 0;
							break;
					}

					return;
				case float64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int16 == B->value.float64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int16 >  B->value.float64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int16 <  B->value.float64 ? 255 : 0;
							break;
					}

					return;
				case uint64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int16 == B->value.uint64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int16 >  B->value.uint64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int16 <  B->value.uint64 ? 255 : 0;
							break;
					}

					return;
				case int64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int16 == B->value.int64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int16 >  B->value.int64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int16 <  B->value.int64 ? 255 : 0;
							break;
					}

					return;
			}

			break;
		case float32:
			switch ( B->mode ){
				case uint8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float32 == B->value.uint8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float32 >  B->value.uint8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float32 <  B->value.uint8 ? 255 : 0;
							break;
					}

					return;
				case int8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float32 == B->value.int8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float32 >  B->value.int8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float32 <  B->value.int8 ? 255 : 0;
							break;
					}

					return;
				case uint16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float32 == B->value.uint16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float32 >  B->value.uint16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float32 <  B->value.uint16 ? 255 : 0;
							break;
					}

					return;
				case int16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float32 == B->value.int16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float32 >  B->value.int16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float32 <  B->value.int16 ? 255 : 0;
							break;
					}

					return;
				case float32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float32 == B->value.float32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float32 >  B->value.float32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float32 <  B->value.float32 ? 255 : 0;
							break;
					}

					return;
				case uint32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float32 == B->value.uint32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float32 >  B->value.uint32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float32 <  B->value.uint32 ? 255 : 0;
							break;
					}

					return;
				case int32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float32 == B->value.int32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float32 >  B->value.int32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float32 <  B->value.int32 ? 255 : 0;
							break;
					}

					return;
				case float64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float32 == B->value.float64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float32 >  B->value.float64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float32 <  B->value.float64 ? 255 : 0;
							break;
					}

					return;
				case uint64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float32 == B->value.uint64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float32 >  B->value.uint64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float32 <  B->value.uint64 ? 255 : 0;
							break;
					}

					return;
				case int64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float32 == B->value.int64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float32 >  B->value.int64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float32 <  B->value.int64 ? 255 : 0;
							break;
					}

					return;
			}

			break;
		case uint32:
			switch ( B->mode ){
				case uint8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint32 == B->value.uint8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint32 >  B->value.uint8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint32 <  B->value.uint8 ? 255 : 0;
							break;
					}

					return;
				case int8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint32 == B->value.int8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint32 >  B->value.int8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint32 <  B->value.int8 ? 255 : 0;
							break;
					}

					return;
				case uint16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint32 == B->value.uint16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint32 >  B->value.uint16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint32 <  B->value.uint16 ? 255 : 0;
							break;
					}

					return;
				case int16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint32 == B->value.int16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint32 >  B->value.int16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint32 <  B->value.int16 ? 255 : 0;
							break;
					}

					return;
				case float32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint32 == B->value.float32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint32 >  B->value.float32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint32 <  B->value.float32 ? 255 : 0;
							break;
					}

					return;
				case uint32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint32 == B->value.uint32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint32 >  B->value.uint32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint32 <  B->value.uint32 ? 255 : 0;
							break;
					}

					return;
				case int32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint32 == B->value.int32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint32 >  B->value.int32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint32 <  B->value.int32 ? 255 : 0;
							break;
					}

					return;
				case float64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint32 == B->value.float64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint32 >  B->value.float64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint32 <  B->value.float64 ? 255 : 0;
							break;
					}

					return;
				case uint64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint32 == B->value.uint64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint32 >  B->value.uint64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint32 <  B->value.uint64 ? 255 : 0;
							break;
					}

					return;
				case int64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint32 == B->value.int64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint32 >  B->value.int64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint32 <  B->value.int64 ? 255 : 0;
							break;
					}

					return;
			}

			break;
		case int32:
			switch ( B->mode ){
				case uint8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int32 == B->value.uint8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int32 >  B->value.uint8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int32 <  B->value.uint8 ? 255 : 0;
							break;
					}

					return;
				case int8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int32 == B->value.int8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int32 >  B->value.int8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int32 <  B->value.int8 ? 255 : 0;
							break;
					}

					return;
				case uint16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int32 == B->value.uint16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int32 >  B->value.uint16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int32 <  B->value.uint16 ? 255 : 0;
							break;
					}

					return;
				case int16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int32 == B->value.int16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int32 >  B->value.int16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int32 <  B->value.int16 ? 255 : 0;
							break;
					}

					return;
				case float32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int32 == B->value.float32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int32 >  B->value.float32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int32 <  B->value.float32 ? 255 : 0;
							break;
					}

					return;
				case uint32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int32 == B->value.uint32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int32 >  B->value.uint32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int32 <  B->value.uint32 ? 255 : 0;
							break;
					}

					return;
				case int32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int32 == B->value.int32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int32 >  B->value.int32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int32 <  B->value.int32 ? 255 : 0;
							break;
					}

					return;
				case float64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int32 == B->value.float64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int32 >  B->value.float64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int32 <  B->value.float64 ? 255 : 0;
							break;
					}

					return;
				case uint64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int32 == B->value.uint64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int32 >  B->value.uint64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int32 <  B->value.uint64 ? 255 : 0;
							break;
					}

					return;
				case int64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int32 == B->value.int64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int32 >  B->value.int64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int32 <  B->value.int64 ? 255 : 0;
							break;
					}

					return;
			}

			break;
		case float64:
			switch ( B->mode ){
				case uint8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float64 == B->value.uint8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float64 >  B->value.uint8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float64 <  B->value.uint8 ? 255 : 0;
							break;
					}

					return;
				case int8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float64 == B->value.int8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float64 >  B->value.int8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float64 <  B->value.int8 ? 255 : 0;
							break;
					}

					return;
				case uint16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float64 == B->value.uint16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float64 >  B->value.uint16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float64 <  B->value.uint16 ? 255 : 0;
							break;
					}

					return;
				case int16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float64 == B->value.int16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float64 >  B->value.int16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float64 <  B->value.int16 ? 255 : 0;
							break;
					}

					return;
				case float32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float64 == B->value.float32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float64 >  B->value.float32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float64 <  B->value.float32 ? 255 : 0;
							break;
					}

					return;
				case uint32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float64 == B->value.uint32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float64 >  B->value.uint32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float64 <  B->value.uint32 ? 255 : 0;
							break;
					}

					return;
				case int32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float64 == B->value.int32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float64 >  B->value.int32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float64 <  B->value.int32 ? 255 : 0;
							break;
					}

					return;
				case float64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float64 == B->value.float64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float64 >  B->value.float64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float64 <  B->value.float64 ? 255 : 0;
							break;
					}

					return;
				case uint64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float64 == B->value.uint64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float64 >  B->value.uint64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float64 <  B->value.uint64 ? 255 : 0;
							break;
					}

					return;
				case int64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.float64 == B->value.int64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.float64 >  B->value.int64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.float64 <  B->value.int64 ? 255 : 0;
							break;
					}

					return;
			}

			break;
		case uint64:
			switch ( B->mode ){
				case uint8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint64 == B->value.uint8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint64 >  B->value.uint8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint64 <  B->value.uint8 ? 255 : 0;
							break;
					}

					return;
				case int8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint64 == B->value.int8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint64 >  B->value.int8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint64 <  B->value.int8 ? 255 : 0;
							break;
					}

					return;
				case uint16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint64 == B->value.uint16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint64 >  B->value.uint16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint64 <  B->value.uint16 ? 255 : 0;
							break;
					}

					return;
				case int16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint64 == B->value.int16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint64 >  B->value.int16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint64 <  B->value.int16 ? 255 : 0;
							break;
					}

					return;
				case float32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint64 == B->value.float32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint64 >  B->value.float32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint64 <  B->value.float32 ? 255 : 0;
							break;
					}

					return;
				case uint32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint64 == B->value.uint32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint64 >  B->value.uint32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint64 <  B->value.uint32 ? 255 : 0;
							break;
					}

					return;
				case int32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint64 == B->value.int32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint64 >  B->value.int32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint64 <  B->value.int32 ? 255 : 0;
							break;
					}

					return;
				case float64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint64 == B->value.float64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint64 >  B->value.float64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint64 <  B->value.float64 ? 255 : 0;
							break;
					}

					return;
				case uint64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint64 == B->value.uint64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint64 >  B->value.uint64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint64 <  B->value.uint64 ? 255 : 0;
							break;
					}

					return;
				case int64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.uint64 == B->value.int64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.uint64 >  B->value.int64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.uint64 <  B->value.int64 ? 255 : 0;
							break;
					}

					return;
			}

			break;
		case int64:
			switch ( B->mode ){
				case uint8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int64 == B->value.uint8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int64 >  B->value.uint8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int64 <  B->value.uint8 ? 255 : 0;
							break;
					}

					return;
				case int8:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int64 == B->value.int8 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int64 >  B->value.int8 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int64 <  B->value.int8 ? 255 : 0;
							break;
					}

					return;
				case uint16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int64 == B->value.uint16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int64 >  B->value.uint16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int64 <  B->value.uint16 ? 255 : 0;
							break;
					}

					return;
				case int16:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int64 == B->value.int16 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int64 >  B->value.int16 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int64 <  B->value.int16 ? 255 : 0;
							break;
					}

					return;
				case float32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int64 == B->value.float32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int64 >  B->value.float32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int64 <  B->value.float32 ? 255 : 0;
							break;
					}

					return;
				case uint32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int64 == B->value.uint32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int64 >  B->value.uint32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int64 <  B->value.uint32 ? 255 : 0;
							break;
					}

					return;
				case int32:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int64 == B->value.int32 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int64 >  B->value.int32 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int64 <  B->value.int32 ? 255 : 0;
							break;
					}

					return;
				case float64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int64 == B->value.float64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int64 >  B->value.float64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int64 <  B->value.float64 ? 255 : 0;
							break;
					}

					return;
				case uint64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int64 == B->value.uint64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int64 >  B->value.uint64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int64 <  B->value.uint64 ? 255 : 0;
							break;
					}

					return;
				case int64:
					switch (static_cast<Comparason>(act->param[1])){
						case equal:
							C->value.uint64 = A->value.int64 == B->value.int64 ? 255 : 0;
							break;
						case greater:
							C->value.uint64 = A->value.int64 >  B->value.int64 ? 255 : 0;
							break;
						case less:
							C->value.uint64 = A->value.int64 <  B->value.int64 ? 255 : 0;
							break;
					}

					return;
			}

			break;
	}

	std::cerr << "Error: Unexpected error, unhandled type comparason case" << std::endl;
	std::cerr << "  A: " << A->mode << std::endl;
	std::cerr << "  B: " <<  B->mode << std::endl;
	std::cerr << "  C: " << C->mode << std::endl;
	std::cerr << "  line: " << act->line;
}
void Instance::CmdBit       (Action *act){
	uint64_t A;
	uint64_t B;

	// The first term can either be a constant or a register
	if (act->param[0] == 1){
		A = this->handle[ act->param[1] ].value.uint64;
	}else{
		A = act->param[1];
	}

	// The second term can either be a constant or a register
	if (act->param[3] == 1){
		B = this->handle[ act->param[4] ].value.uint64;
	}else{
		B = act->param[4];
	}

	// Apply correct behaviour for the operation
	switch( static_cast<BitOperator>( act->param[2] ) ){
		case AND:
			this->handle[ act->param[5] ].value.uint64 = A&B;
			break;
		case OR:
			this->handle[ act->param[5] ].value.uint64 = A|B;
			break;
		case XOR:
			this->handle[ act->param[5] ].value.uint64 = A^B;
			break;
		case LeftShift:
			this->handle[ act->param[5] ].value.uint64 = A << B;
			break;
		case RightShift:
			this->handle[ act->param[5] ].value.uint64 = A >> B;
			break;
		case NOT:
			this->handle[ act->param[5] ].value.uint64 = ~A;
			break;
	}
};
void Instance::CmdLComp     (Action *act){
	// Establish pointers for the currently comparing bytes
	char *A = reinterpret_cast<char *>( this->handle[ act->param[0] ].value.address );
	char *B = reinterpret_cast<char *>( this->handle[ act->param[2] ].value.address );

	// Establish how many bytes will be compared
	unsigned long length = act->param[2];

	// Little edian (right to left)
	if (act->param[4] == 0){
		A += length;
		B += length;

		while (length > 0){
			// If the bytes are different
			//   Store result based on desired action
			if (A > B){
				// Store true when finding greater than, since it is
				this->handle[ act->param[3] ].value.uint8 = (act->param[1] == Comparason::greater);

				// Stop comparing since a difference has been found
				return;
			}else if (A < B){
				// Store true when finding less than, since it is
				this->handle[ act->param[3] ].value.uint8 = (act->param[1] == Comparason::less);

				// Stop comparing since a difference has been found
				return;
			}

			length -= 1;
			A--;
			B--;
		}

		this->handle[ act->param[3] ].value.uint8 = (act->param[1] == Comparason::equal);
	}else{ // Big edian (left to right)
		while (length > 0){
			// If the bytes are different
			//   Store result based on desired action
			if (A > B){
				// Store true when finding greater than, since it is
				this->handle[ act->param[3] ].value.uint8 = (act->param[1] == Comparason::greater);

				// Stop comparing since a difference has been found
				return;
			}else if (A < B){
				// Store true when finding less than, since it is
				this->handle[ act->param[3] ].value.uint8 = (act->param[1] == Comparason::less);

				// Stop comparing since a difference has been found
				return;
			}

			length -= 1;
			A++;
			B++;
		}

		this->handle[ act->param[5] ].value.uint8 = (act->param[1] == Comparason::equal);
	}
};
unsigned long Instance::CmdInstance  (Action *act, unsigned long cursor){
	Function *funcTarget = nullptr;
	Instance *target = nullptr;
	Thread::Job task;
	unsigned int length;

	bool found = false;

	std::string msg;

	switch (act->param[0]){
		case 0: // create
			funcTarget = this->ref;
			target = this;
			length = act->param[3];

			// Put cursors in correct scope
			while (length > 0){
				if (funcTarget->parent == nullptr){
					// Construct the string as one big pipable object
					// So that another thread will not intercept mid print
					msg = "Error: Unexpected function structure failure\n\r";
					msg += "  line: " + std::to_string(act->line);
					std::cerr << msg << std::endl;
					return 0;
				}
				if (target->parent == nullptr){
					// Construct the string as one big pipable object
					// So that another thread will not intercept mid print
					msg = "Error: Unexpected instance structure failure\n\r";
					msg += "  line: " + std::to_string(act->line);
					std::cerr << msg << std::endl;
					return 0;
				}

				// Itterate up the tree
				funcTarget = funcTarget->parent;
				target = target->parent;
				length--;
			}

			// Now that we are at the correct depth
			//   Select the correct child
			if (act->param[1] >= funcTarget->child.size()){
				// Construct the string as one big pipable object
				// So that another thread will not intercept mid print
				msg = "Error: Unexpected function structure failure. Invalid child ID\n\r";
				msg += "  line: " + std::to_string(act->line);
				std::cerr << msg << std::endl;
				return 0;
			}
			funcTarget = funcTarget->child[ act->param[1] ];


			// Find an empty space to create a new instance reference
			length = this->child.size();
			found = false;
			for (unsigned int i=0; i<length; i++){
				if (this->child[i] == nullptr){
					this->child[i] = new Instance(
						funcTarget,                         // Code      (reference)
						this,                               // Callback  (reference)
						this->pool,                         // Execution (reference)
						target                              // Scope     (reference)
					);
					target = this->child[i];
					found  = true;
					break;
				}
			}

			// Was unable to find an empty slot
			if (!found){
				this->child.resize(length+1);

				this->child[length] = new Instance(
					funcTarget,                         // Code      (reference)
					this,                               // Callback  (reference)
					this->pool,                         // Execution (reference)
					target                              // Scope     (reference)
				);
				target = this->child[length];
			}

			this->handle[ act->param[2] ].value.address = target;
			return 0;
		case 1: // yeild
			target = reinterpret_cast<Instance*>( this->handle[ act->param[1] ].value.address );

			target->yeildPos = cursor + 1;
			return act->param[2]+1;
		case 2: // return
			target = reinterpret_cast<Instance*>( this->handle[ act->param[1] ].value.address );

			target->returnPos = cursor + 1;
			return act->param[2]+1;
		case 3: // execute
			target = reinterpret_cast<Instance*>( this->handle[ act->param[1] ].value.address );

			task.ptr = target;
			task.cursor = 0;

			// Initial execution does not parse data
			task.data = nullptr;
			task.size = 0;

			if (act->param[2] == 1){ // Hand the task to worker 0
				this->pool->Dispatch(task, true, 0);
			}else{
				this->pool->Dispatch(task, false, 0);
			}

			return 0;
		case 4: // parent
			this->handle[ act->param[0] ].value.address = this->parent;
			return 0;
	}

	return 0;
};
void Instance::CmdLocal     (Action *act){
	Instance *target;

	switch(act->param[1]){
		case 0:
			target = this;
			break;
		case 1:
			target = reinterpret_cast<Instance*>(this->handle[ act->param[2] ].value.address);
			break;
	}

	this->handle[ act->param[0] ].value.address = target->GetLocalSpace();
};
void Instance::CmdReturn    (Action *act){
	// Don't return if execution should of halted
	if (this->HasDestoryed() == true){
		return;
	}

	// If returing is impossible
	//  Root function has no parent
	if (this->caller == nullptr){
		return;
	}

	// No return action bound
	if (this->returnPos == 0){
		std::string msg;
		msg = "Instance: no return position\n";
		std::cout << msg;

		this->Destory();
		return;
	}

	Thread::Job task;
	task.ptr = this->caller;
	task.cursor = this->returnPos;

	// Returning does not parse data
	task.data = nullptr;
	task.size = 0;

	// Queue callback code
	this->pool->Dispatch(task, true, this->caller->workerID);

	// Stop this instance from doing anymore work as it has now returned
	this->Destory();
	return;
};
void Instance::CmdYeild     (Action *act){
	// Don't return if execution should of halted
	if (this->HasDestoryed() == true){
		return;
	}

	// If returing is impossible
	//  Root function has no parent
	if (this->caller == nullptr){
		return;
	}

	// No yeild action bound
	if (this->yeildPos == 0){
		std::string msg;
		msg = "Instance: no yeild position\n";
		std::cout << msg;

		return;
	}

	Thread::Job task;
	task.ptr = this->caller;
	task.cursor = this->yeildPos;

	// Move the yeild result to a safe space
	// Parse the data for the callback to use
	task.size = this->ref->resultSize;
	task.data = Memory::Allocate(task.size);
	Memory::Duplicate(
		task.data,
		this->handle[ act->param[0] ].value.address,
		task.size
	);

	// Queue callback code
	this->pool->Dispatch(task, true, this->caller->workerID);
	return;
};
void Instance::CmdStringify (Action *act){
	std::string str;
	void *to;
	size_t size;

	// Get string information
	str = this->handle[ act->param[0] ].toString();
	size = str.size();

	// Move to safe memory address
	to = Memory::Allocate(size);
	Memory::Duplicate(to, &(str[0]), size);

	// Write information to register
	this->handle[ act->param[1] ].value.address = to;
	this->handle[ act->param[2] ].value.uint64  = size;


	std::string msg;
	msg = "stringify: " + str + "\n";
	std::cout << msg;
};
void Instance::CmdWrite     (Action *act){
	// Read transfer description

	Memory::Duplicate(
		this->handle[ act->param[0] ].value.address,
		&(act->param[2]),
		act->param[1]
	);
}
