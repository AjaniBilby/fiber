#include "./table.hpp"

Table::Table(std::vector<RawAction> tokens){
	this->valid = true;

	// Raise function definitions
	std::vector<RawAction> forward;
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
			this->function.push_back( Function(tokens[i].param[1], forward, lsize.data.uint64, this) );
			i = j;
			continue;
		}


		std::cerr << "Error: Attempting command out of function scope." << std::endl;
		std::cerr << "       All commands must execute within the scope of a function" << std::endl;
		std::cerr << "  line    : " << tokens[i].line << std::endl;
		std::cerr << "  command : " << tokens[i].param[0] << std::endl;
		std::cerr << "    " << ToString(tokens[i]) << std::endl;
		this->valid = false;
	}
}



Function* Table::Find(std::string str){
	size_t size = this->function.size();
	for (size_t i=0; i<size; i++){
		if (this->function[i].name == str){
			return &this->function[i];
		}
	}

	return nullptr;
};
