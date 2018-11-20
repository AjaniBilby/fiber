#include "./function.hpp"

Function::Function(std::string name, std::vector<RawAction> tokens, size_t domainSize, Function* owner){
	this->domain = domainSize;
	this->parent = owner;
	this->valid = true;
	this->name = name;

	// Raise function definitions
	std::vector<RawAction> next;
	size_t size = tokens.size();
	for (size_t i=0; i<size; i++){
		// Forward any child functions through to a new child
		if (tokens[i].param[0] == "func"){
			std::vector<RawAction> forward;
			unsigned long depth = 0;

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
			if (tokens[j].param[0] != "}"){
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
	std::vector<Action> cmd;
	tokens = next;
	size = tokens.size();
	for (size_t i=0; i<size; i++){
		Action temp;
		temp = Interpreter::Convert(tokens[i], this);

		if (temp.cmd == Command::invalid){
			this->valid = false;
			return;
		}

		cmd.push_back(temp);
	}


	// Compact the code
	size = cmd.size();
	for (size_t i=0; i<size; i++){
		this->code.append(cmd[i]);
	}
	this->code.simplifyJumps();



	// Check if any child is invalid
	// If so make this function invalid
	size = this->child.size();
	for (unsigned long i=0; i<size; i++){
		if (this->child[i].valid == false){
			this->valid = false;
			return;
		}
	}
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
