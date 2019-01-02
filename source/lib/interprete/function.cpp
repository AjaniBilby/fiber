#include "./function.hpp"

Function::Function(std::string name, std::vector<RawAction> tokens, size_t domainSize, Function* owner){
	this->domain = domainSize;
	this->parent = owner;
	this->valid = true;
	this->name = name;

	// Raise function definitions
	std::vector<RawAction> forward;
	std::vector<RawAction> next;
	size_t size = tokens.size();
	for (size_t i=0; i<size; i++){
		// Forward any child functions through to a new child
		if (tokens[i].param[0] == "func"){
			unsigned long depth = 0;
			forward.resize(0);

			if (tokens[i].param.size() != 3){
				std::cerr << "Error: Invalid number of arguments during function definition" << std::endl;
				std::cerr << "  line: " << tokens[i].line << std::endl;
				this->valid = false;
				break;
			}

			// Read the function's domain size
			auto lsize = Interpreter::Opperand(tokens[i].param[2]);
			if ( lsize.valid == false || (lsize.type != Interpreter::OpperandType::Bytes && lsize.type != Interpreter::OpperandType::Uint) ){
				std::cerr << "Error: Invalid function domain size. Domain size must be specified via a hex or uint value" << std::endl;
				std::cerr << "    arg: " << tokens[i].param[2] << std::endl;
				std::cerr << "   type: " << lsize.typeToString() << std::endl;
				std::cerr << "  valid: " << lsize.valid << std::endl;
				std::cerr << "   line: " << tokens[i].line << std::endl;
				this->valid = false;
				break;
			}

			// If the next line is not an open bracket
			if (i+1 >= size || tokens[i+1].param[0] != "{"){
				std::cerr << "Error: Invalid function definition, missing opening bracket on new line" << std::endl;
				std::cerr << "  line: " << tokens[i].line << std::endl;
				this->valid = false;
				break;
			}

			// Find the closing bracket at the same depth
			forward.resize(0); // tokens between the brackets
			depth = 1;
			unsigned long j=i+2;
			for (; j<size; j++){
				if (tokens[j].param[0] == "{"){
					depth++;
				}else if (tokens[j].param[0] == "}"){
					depth--;
				}

				if (depth == 0){
					break;
				}

				forward.push_back(tokens[j]);
			}

			// If an end bracket was not able to be found
			if (j >= size || tokens[j].param[0] != "}"){
				std::cerr << "Error: Invalid function definition, missing closing bracket" << std::endl;
				std::cerr << "  line: " << tokens[i].line << std::endl;
				this->valid = false;
				break;
			}

			// Parse the gathered information forward to a child function
			this->child.push_back( Function(tokens[i].param[1], forward, lsize.data.uint64, this) );
			i = j;
			continue;
		}


		next.push_back(tokens[i]);
	}

	// Interpret raw tokens
	size = next.size();
	this->actions.resize(size);
	for (size_t i=0; i<size; i++){
		this->actions[i] = Interpreter::Convert(next[i], this);

		if (this->actions[i].cmd == Command::invalid){
			this->valid = false;
			return;
		}
	}
	tokens.resize(0);
	next.resize(0);

	this->SimplifyBehaviour();

	size = this->child.size();
	for (size_t i=0; i<size; i++){
		this->child[i].finalize();
	}

	return;
};





void Function::SimplifyBehaviour(){
	this->SimplifyBehaviour_initilize();
	this->SimplifyBehaviour_if();
	this->SimplifyBehaviour_loop();
};

void Function::SimplifyBehaviour_if(){
	// See: docs/internals/commands/if

	size_t size = this->actions.size();
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
}

void Function::SimplifyBehaviour_initilize(){
	// See: docs/internals/commands/initilize

	size_t size = this->actions.size();
	for (size_t i=0; i<size; i++){
		if (this->actions[i].cmd == Command::initilize){

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


			// Find the end of the preamble
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
			this->actions[i+1].param[0] = (j+2);
			this->actions[j].cmd = Command::jump;
			this->actions[j].param.resize(1);
			this->actions[j].param[0] = (k+2);
			this->actions[j+1].cmd = Command::stop;
			this->actions[j+1].param.resize(0);
			this->actions[k].cmd = Command::stop;
			this->actions[k].param.resize(0);
		}
	}
}

void Function::SimplifyBehaviour_loop(){
	std::vector< __BlockStackElement__ > stack;
	size_t stackSize = 0;

	size_t size = this->actions.size();
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
			this->actions[i].param[0] = stack[ stackSize - ( this->actions[i].param[0] + 1) ].start;
			this->actions[i].param.resize(1);
		}
	}
}





void Function::finalize(){
	size_t size = this->actions.size();
	for (size_t i=0; i<size; i++){
		this->code.append(this->actions[i]);
	}

	this->code.simplify();

	this->actions.resize(0);
};





FunctionReference Function::find(std::string str){
	FunctionReference out;
	out.relDepth = 0;

	// Search children
	size_t children = this->child.size();
	for (size_t i=0; i<children; i++){
		if (this->child[i].name == str){
			out.ptr = &this->child[i];
			return out;
		}
	}

	// No children have that name
	// Search up the definition stack
	if (this->parent != nullptr){
		out = this->parent->find(str);
		out.relDepth++;

		return out;
	}

	// Unable to search up the definition stack as this is the apparent top
	out.ptr = nullptr;
	return out;
}
