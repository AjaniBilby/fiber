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
	actions.resize(size);
	for (size_t i=0; i<size; i++){
		actions[i] = Interpreter::Convert(next[i], this);

		if (actions[i].cmd == Command::invalid){
			this->valid = false;
			return;
		}
	}
	tokens.resize(0);
	next.resize(0);

	// Simplify initilize behaviour
	//  See: docs/internals/commands/initilize
	for (size_t i=0; i<size; i++){
		if (actions[i].cmd == Command::initilize){

			// Check it has a code block after it
			if (i+1 >= size || actions[i+1].cmd != Command::blockOpen){
				std::cerr << "Error: Invalid initilization; missing preamble." << std::endl;
				std::cerr << "  line: " << actions[i].line << std::endl;

				this->valid = false;
				return;
			}


			// Find the end of the preamble
			size_t j=i+1;
			size_t depth = 0;
			for (; j<size; j++){
				if (actions[j].cmd == Command::blockOpen){
					depth++;
				}
				if (actions[j].cmd == Command::blockClose){
					depth--;
				}

				if (depth == 0){
					break;
				}
			}
			if (j>=size || actions[j].cmd != Command::blockClose){
				std::cerr << "Error: Invalid initilization; unable to find the end of preamble." << std::endl;
				std::cerr << "  line: " << actions[i].line << std::endl;

				this->valid = false;
				return;
			}


			// Check for the second code block
			if (j+1 >= size || actions[j+1].cmd != Command::blockOpen){
				std::cerr << "Error : Invalid initilization; missing finishing code." << std::endl;
				std::cerr << " line: " << actions[i].line << std::endl;

				this->valid = false;
				return;
			}


			// Find the end of the preamble
			size_t k=j+1;
			depth = 0;
			for (; k<size; k++){
				if (actions[k].cmd == Command::blockOpen){
					depth++;
				}
				if (actions[k].cmd == Command::blockClose){
					depth--;
				}

				if (depth == 0){
					break;
				}
			}
			if (k>=size || actions[k].cmd != Command::blockClose){
				std::cerr << "Error: Invalid initilization; unable to find the end of preamble." << std::endl;
				std::cerr << "  line: " << actions[i].line << std::endl;

				this->valid = false;
				return;
			}
			actions[k].cmd = Command::stop;


			// Convert the necessary brackets
			actions[i+1].cmd = Command::jump;
			actions[i+1].param.resize(1);
			actions[i+1].param[0] = (j+2);
			actions[j].cmd = Command::jump;
			actions[j].param.resize(1);
			actions[j].param[0] = (k+2);
			actions[j+1].cmd = Command::stop;
			actions[j+1].param.resize(0);
			actions[k].cmd = Command::stop;
			actions[k].param.resize(0);
		}
	}

	// Simplify if behaviour
	//  See: docs/internals/commands/if
	size = actions.size();
	for (size_t i=0; i<size; i++){
		if (actions[i].cmd == Command::gate){

			// Check an opening bracket exists
			if (i+1 >= size || actions[i+1].cmd != Command::blockOpen){
				std::cerr << "Error: If statement is missing opening bracket on new line" << std::endl;
				std::cerr << "  line: " << actions[i].line << std::endl;

				this->valid = false;
				return;
			}

			// Find the closing bracket
			size_t j=i+1;   // starts on the opening bracket after the IF
			size_t depth=0;
			for (; j<size; j++){
				if (actions[j].cmd == Command::blockOpen){
					depth++;
				}
				if (actions[j].cmd == Command::blockClose){
					depth--;
				}

				if (depth == 0){
					break;
				}
			}
			if (j >= size || actions[j].cmd != Command::blockClose){
				std::cerr << "Error: Missing if statement closing bracket" << std::endl;
				std::cerr << "  line: " << actions[i].line;

				this->valid = false;
				return;
			}

			if (j+1 < size && actions[j+1].cmd == Command::gateOther){ // If there is an else clause
				// Check the else clause has an open bracket
				if (j+2 < size || actions[j+2].cmd != Command::blockOpen){
					std::cerr << "Error: Invalid else clause, missing opening bracket." << std::endl;
					std::cerr << "  line: " << actions[j+1].line << std::endl;

					this->valid = false;
					return;
				}

				// Find else closing bracket
				size_t k=j+2; // start at the opening bracket after the else
				depth = 0;
				for (; k<size; k++){
					if (actions[k].cmd == Command::blockOpen){
						depth++;
					}
					if (actions[k].cmd == Command::blockClose){
						depth--;
					}

					if (depth == 0){
						break;
					}
				}
				if (k>=size || actions[k].cmd != Command::blockClose){
					std::cerr << "Error: Unable to find the closing bracket for else clause." << std::endl;
					std::cerr << "  line: " << actions[j+2].line << std::endl;

					this->valid = false;
					return;
				}

				// Convert necessary commands
				actions[i+1].cmd = Command::jump;
				actions[i+1].param.resize(1);
				actions[i+1].param[0] = (j+3);
				actions[j].cmd = Command::jump;
				actions[j].param.resize(1);
				actions[j].param[0] = (k+1);
				actions[j+1].cmd = Command::blank;
				actions[j+1].param.resize(0);
				actions[j+2].cmd = Command::blank;
				actions[j+2].param.resize(0);
				actions[k].cmd = Command::blank;
				actions[k].param.resize(0);
			}else{ // Has no else clause
				actions[i+1].cmd = Command::jump;
				actions[i+1].param.resize(1);
				actions[i+1].param[0] = (j+1) - (i+1);
				actions[j].cmd = Command::blank;
				actions[j].param.resize(0);
			}
		}
	}


	size = this->child.size();
	for (size_t i=0; i<size; i++){
		this->child[i].finalize();
	}

	return;
};


void Function::finalize(){
	size_t size = actions.size();
	std::cout << "Pushing " << size << std::endl;
	for (size_t i=0; i<size; i++){
		std::cout << " " << ToString(actions[i]) << std::endl;
		this->code.append(actions[i]);
	}

	std::cout << "  Next" << std::endl;
	auto ptr = this->code.next();
	while (ptr != nullptr){
		std::cout << "   " << ToString(ptr) << std::endl;
		ptr = this->code.next(ptr);
	}

	std::cout << "Simplifying" << std::endl;
	this->code.simplify();

	ptr = this->code.next();
	while (ptr != nullptr){
		std::cout << "  " << ToString(ptr) << std::endl;
		ptr = this->code.next(ptr);
	}

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
