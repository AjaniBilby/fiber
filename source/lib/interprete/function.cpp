#include "./function.hpp"

Function::Function(std::string name, std::vector<RawAction> tokens, size_t domainSize, void* owner){
	this->domain = domainSize;
	this->parent = owner;
	this->valid = true;
	this->name = name;

	// Interpret raw tokens
	size_t size = tokens.size();
	this->actions.resize(size);
	for (size_t i=0; i<size; i++){
		this->actions[i] = Interpreter::Convert(tokens[i], this);

		if (this->actions[i].cmd == Command::invalid){
			this->valid = false;
			return;
		}
	}
	tokens.resize(0);


	// Convert complex bahaviours into simpler ones
	//   (e.i. loops into jumps)
	this->SimplifyBehaviour();


	// Move commands into bytecode
	for (size_t i=0; i<size; i++){
		this->code.append(this->actions[i]);
	}

	// Removes blanks and simplifies jumps
	this->code.simplify();

	// Delete the temporary state of the commands used during simplifying behaviour
	this->actions.resize(0);
	return;
};





void Function::SimplifyBehaviour(){
	size_t size = this->actions.size();

	// See: docs/internals/commands/initilize
	for (size_t i=0; i<size; i++){
		if (this->actions[i].cmd == Command::initilize){

			// Get an exact funciton reference by name
			std::string* name = reinterpret_cast<std::string*>(this->actions[i].param[0]);
			auto ref = reinterpret_cast<Table*>(this->parent)->Find(*name);
			if (ref == nullptr){
				std::cerr << "Error: Invalid function call due to invalid name." << std::endl;
				std::cerr << "  line: " << this->actions[i].line << std::endl;
				std::cerr << "  name: " << name << std::endl;
			}
			this->actions[i].param[0] = reinterpret_cast<uint64>(ref);
			delete name;

			// Check it has a code block after it
			if (i+1 >= size || this->actions[i+1].cmd != Command::blockOpen){
				std::cerr << "Error: Invalid initilization; missing preamble." << std::endl;
				std::cerr << "  line: " << this->actions[i].line << std::endl;

				this->valid = false;
				return;
			}


			// Find the end of the preamble
			size_t j=i+1;
			size_t depth = 0;
			for (; j<size; j++){
				if (this->actions[j].cmd == Command::blockOpen){
					depth++;
				}
				if (this->actions[j].cmd == Command::blockClose){
					depth--;
				}

				if (depth == 0){
					break;
				}
			}
			if (j>=size || this->actions[j].cmd != Command::blockClose){
				std::cerr << "Error: Invalid initilization; unable to find the end of preamble." << std::endl;
				std::cerr << "  line: " << this->actions[i].line << std::endl;

				this->valid = false;
				return;
			}


			// Check for the second code block
			if (j+1 >= size || this->actions[j+1].cmd != Command::blockOpen){
				std::cerr << "Error : Invalid initilization; missing finishing code." << std::endl;
				std::cerr << " line: " << this->actions[i].line << std::endl;

				this->valid = false;
				return;
			}


			// Find the end of the return code
			size_t k=j+1;
			depth = 0;
			for (; k<size; k++){
				if (this->actions[k].cmd == Command::blockOpen){
					depth++;
				}
				if (this->actions[k].cmd == Command::blockClose){
					depth--;
				}

				if (depth == 0){
					break;
				}
			}
			if (k>=size || this->actions[k].cmd != Command::blockClose){
				std::cerr << "Error: Invalid initilization; unable to find the end of preamble." << std::endl;
				std::cerr << "  line: " << this->actions[i].line << std::endl;

				this->valid = false;
				return;
			}
			this->actions[k].cmd = Command::stop;


			// Convert the necessary brackets
			this->actions[i+1].cmd = Command::jump;
			this->actions[i+1].param.resize(1);
			this->actions[i+1].param[0] = (j+1);
			this->actions[j].cmd = Command::launch;
			this->actions[j].param.resize(0);
			this->actions[j+1].cmd = Command::jump;
			this->actions[j+1].param.resize(1);
			// Jump to after the return code's stop if there is more code afterwards
			// Otherwise just jump to the stop as it is the last command
			if (k+1 >= size){
				this->actions[j+1].param[0] = (k);
			}else{
				this->actions[j+1].param[0] = (k+1);
			}
			this->actions[k].cmd = Command::stop;
			this->actions[k].param.resize(0);
		}
	}



	// See: docs/internals/commands/if
	for (size_t i=0; i<size; i++){
		if (this->actions[i].cmd == Command::gate){

			// Check an opening bracket exists
			if (i+1 >= size || this->actions[i+1].cmd != Command::blockOpen){
				std::cerr << "Error: If statement is missing opening bracket on new line" << std::endl;
				std::cerr << "  line: " << this->actions[i].line << std::endl;

				this->valid = false;
				return;
			}

			// Find the closing bracket
			size_t j=i+1;   // starts on the opening bracket after the IF
			size_t depth=0;
			for (; j<size; j++){
				if (this->actions[j].cmd == Command::blockOpen){
					depth++;
				}
				if (this->actions[j].cmd == Command::blockClose){
					depth--;
				}

				if (depth == 0){
					break;
				}
			}
			if (j >= size || this->actions[j].cmd != Command::blockClose){
				std::cerr << "Error: Missing if statement closing bracket" << std::endl;
				std::cerr << "  line: " << this->actions[i].line;

				this->valid = false;
				return;
			}

			if (j+1 < size && this->actions[j+1].cmd == Command::gateOther){ // If there is an else clause
				// Check the else clause has an open bracket
				if (j+2 >= size || this->actions[j+2].cmd != Command::blockOpen){
					std::cerr << "Error: Invalid else clause, missing opening bracket." << std::endl;
					std::cerr << "  line: " << this->actions[j+1].line << std::endl;

					this->valid = false;
					return;
				}

				// Find else closing bracket
				size_t k=j+2; // start at the opening bracket after the else
				depth = 0;
				for (; k<size; k++){
					if (this->actions[k].cmd == Command::blockOpen){
						depth++;
					}
					if (this->actions[k].cmd == Command::blockClose){
						depth--;
					}

					if (depth == 0){
						break;
					}
				}
				if (k>=size || this->actions[k].cmd != Command::blockClose){
					std::cerr << "Error: Unable to find the closing bracket for else clause." << std::endl;
					std::cerr << "  line: " << this->actions[j+2].line << std::endl;

					this->valid = false;
					return;
				}

				// Convert necessary commands
				this->actions[i+1].cmd = Command::jump;
				this->actions[i+1].param.resize(1);
				this->actions[i+1].param[0] = j+2;
				this->actions[j].cmd = Command::jump;
				this->actions[j].param.resize(1);
				this->actions[j].param[0] = k;
				this->actions[j+1].cmd = Command::blank;
				this->actions[j+1].param.resize(0);
				this->actions[j+2].cmd = Command::blank;
				this->actions[j+2].param.resize(0);
				this->actions[k].cmd = Command::blank;
				this->actions[k].param.resize(0);
			}else{ // Has no else clause
				this->actions[i+1].cmd = Command::jump;
				this->actions[i+1].param.resize(1);
				this->actions[i+1].param[0] = j;
				this->actions[j].cmd = Command::blank;
				this->actions[j].param.resize(0);
			}
		}
	}



	std::vector< __BlockStackElement__ > stack;
	size_t stackSize = 0;
	for (size_t i=0; i<size; i++){

		// Find new loops to add to the stack
		if (this->actions[i].cmd == Command::loop){
			// Check for opening bracket after loop decleartion
			if (i+1 >= size || this->actions[i+1].cmd == Command::blockOpen){
				std::cerr << "Error: Loop is missing an opening bracket" << std::endl;
				std::cerr << "  line: " << this->actions[i].line << std::endl;

				this->valid = false;
				return;
			}

			// Find the ending bracket at the same depth
			size_t depth = 1;
			size_t     k = i+2;
			for (; k<size; k++){
				if (this->actions[k].cmd == Command::blockClose){
					depth--;
				}
				if (this->actions[k].cmd == Command::blockOpen){
					depth++;
				}

				if (depth == 0){
					break;
				}
			}

			if (k >= size || this->actions[k].cmd != Command::blockClose){
				std::cerr << "Error: Loop is missing a closing bracket" << std::endl;
				std::cerr << "  line: " << this->actions[i].line << std::endl;

				this->valid = false;
				return;
			}

			// Mark a new loop in play
			stack.push_back({ i, k });
			stackSize++;
		}

		// Pop the last element if it is now closed
		//   Simplify the behaviour of the loop as well
		if (stackSize > 0){
			size_t k = stackSize-1;
			if ( i >= stack[k].end ){
				// Erase the loop tag
				this->actions[ stack[k].start ].cmd = Command::blank;
				this->actions[ stack[k].start ].param.resize(0);

				// Erase the opening bracket after the loop tag
				this->actions[ stack[k].start+1 ].cmd = Command::blank;
				this->actions[ stack[k].start+1 ].param.resize(0);

				// Convert the closing bracket to a loop back point
				this->actions[ stack[k].end ].cmd = Command::jump;
				this->actions[ stack[k].end ].param.resize(1);
				this->actions[ stack[k].end ].param[0] = stack[k].start+1;

				// Remove the loop reference from the stack
				stack.resize(k);
			}
		}

		// Convert break behaviour
		if (this->actions[i].cmd == Command::blockExit){
			// Check the depth required is correct
			if (this->actions[i].param[0] >= stackSize){
				std::cerr << "Error: Invalid break command. The depth requested is deeper than actual depth." << std::endl;
				std::cerr << "  line: " << this->actions[i].line << std::endl;

				this->valid = false;
				return;
			}

			// Convert the break to a relevent jump
			this->actions[i].cmd = Command::jump;
			this->actions[i].param[0] = stack[ stackSize - ( this->actions[i].param[0] + 1) ].end;
			this->actions[i].param.resize(1);
		}

		// Convert jump behaviour
		if (this->actions[i].cmd == Command::blockRepeat){
			// Check the depth required is correct
			if (this->actions[i].param[0] >= stackSize){
				std::cerr << "Error: Invalid continue command. The depth requested is deeper than actual depth." << std::endl;
				std::cerr << "  line: " << this->actions[i].line << std::endl;

				this->valid = false;
				return;
			}

			// Convert the break to a relevent jump
			this->actions[i].cmd = Command::jump;
			this->actions[i].param[0] = stack[ stackSize - ( this->actions[i].param[0] + 1) ].start+1;
			this->actions[i].param.resize(1);
		}
	}
};
