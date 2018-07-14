#include "function.hpp"




Function::Function(std::string name, unsigned long local, unsigned long result){
	this->name = name;
	this->localSize = local;
	this->resultSize = result;
	this->code.reserve(0);
}
int Function::GetChildsID(std::string str){
	int length = this->child.size();
	for (int i=0; i<length; i++){
		if (this->child[i]->name == str){
			return i;
		}
	}

	return -1;
};

bool Function::Subdivide(Segregate::StrCommands src){
	if (src[0].param.size() < 4){
		std::cerr << "Error: Not enough function parameters" << std::endl;
		std::cerr << "  line: " << src[0].line << std::endl;
		return false;
	}

	std::string name     = src[0].param[1];
	unsigned long local  = Hexidecimal::convert( src[0].param[2] );
	unsigned long result = Hexidecimal::convert( src[0].param[3] );

	// Organize storage
	unsigned long index = this->child.size();
	this->child.resize(index+1);

	// Parse child properties
	this->child[index] = new Function(name, local, result);

	// Remove the first command (function definition)
	unsigned long length = src.size();
	for (int i=1; i<length; i++){
		src[i-1] = src[i];
	}
	src.resize(length-1);

	return this->child[index]->Parse(src);
};


bool Function::Interpret(Segregate::StrCommands source){
	unsigned int codeLen = source.size();
	this->code.resize( codeLen );

	// Information for breaking up subfunctions
	bool found = false;
	int depth = 0;
	int d2 = 0;

	bool error = false;          // Has an error been detected?
	int ptr = 0;                 // Which index to place the next action


	for (unsigned int i=0; i<codeLen; i++){

		// Ignore comment lines
		//  '#*'
		if (source[i].param[0][0] == '#'){
			continue;
		}
		//  '; *'
		if (source[i].param[0] == ";"){
			continue;
		}

		// Stop
		if (source[i].param[0] == "stop"){
			this->code[ptr].command = Commands::stop;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(0);

			ptr++;
			continue;
		};

		// Set Register
		if (source[i].param[0] == "set"){
			if (source[i].param.size() < 3){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::set;                               // Command ID
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(3);
			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);                // Register to be written

			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Supplied bad target register "<<source[i].param[1] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			if (source[i].param[2][0] == 'r'){
				this->code[ptr].param[1] = 1;
				this->code[ptr].param[2] = GetRegisterID(source[i].param[2]);              // Constant value

				if (this->code[ptr].param[2] == -1){
					std::cerr << "Error: Supplied bad value register "<<source[i].param[2] << std::endl;
					std::cerr << "  line: " << source[i].line << std::endl;
					error = true;
					continue;
				}
			}else{
				this->code[ptr].param[1] = 0;
				this->code[ptr].param[2] = Hexidecimal::convert(source[i].param[2]);       // Constant value
			}

			ptr++;
			continue;
		}

		// Pull data from loc into register
		if (source[i].param[0] == "push"){
			if (source[i].param.size() < 3){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::push;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(2);
			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			this->code[ptr].param[1] = GetRegisterID(source[i].param[2]);

			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Supplied bad value register '"<<source[i].param[1] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}
			if (this->code[ptr].param[1] == -1){
				std::cerr << "Error: Supplied bad address register '"<<source[i].param[2] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}

		// Push register data to loc
		if (source[i].param[0] == "pull"){
			if (source[i].param.size() < 3){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::pull;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(2);
			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			this->code[ptr].param[1] = GetRegisterID(source[i].param[2]);

			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Supplied bad value register '"<<source[i].param[1] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}
			if (this->code[ptr].param[1] == -1){
				std::cerr << "Error: Supplied bad address register '"<<source[i].param[2] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}

		// (Un)Allocate command
		if (source[i].param[0] == "mem"){
			if (source[i].param.size() < 4){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: " << i << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::memory;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(3);

			if (source[i].param[1] == "alloc"){
				this->code[ptr].param[0] = 0;
			}else if (source[i].param[1] == "unalloc"){
				this->code[ptr].param[0] = 1;
			}else{
				error = true;
				std::cerr << "Error: Unknown memory action '" << source[i].param[1] << "'" << std::endl;
				std::cerr << "  line: " << i << std::endl;
				continue;
			}

			this->code[ptr].param[1] = GetRegisterID(source[i].param[2]);
			this->code[ptr].param[2] = GetRegisterID(source[i].param[3]);

			if (this->code[ptr].param[1] == -1){
				std::cerr << "Error: Supplied bad address register '"<<source[i].param[2] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}
			if (this->code[ptr].param[1] == -1){
				std::cerr << "Error: Supplied bad length register '"<<source[i].param[3] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}

		// Standard Stream
		if (source[i].param[0] == "ss"){
			if (source[i].param.size() < 4){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::standardStream;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(3);

			// Mode
			if (source[i].param[1] == "in"){
				this->code[ptr].param[0] = 0;
			}else if (source[i].param[1] == "out"){
				this->code[ptr].param[0] = 1;
			}else if (source[i].param[1] == "err"){
				this->code[ptr].param[0] = 2;
			}else if (source[i].param[1] == "log"){
				this->code[ptr].param[0] = 3;
			}

			this->code[ptr].param[1] = GetRegisterID(source[i].param[2]);
			this->code[ptr].param[2] = GetRegisterID(source[i].param[3]);
			if (this->code[ptr].param[1] == -1){
				std::cerr << "Error: Supplied bad address register '"<<source[i].param[2] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}
			if (this->code[ptr].param[2] == -1){
				std::cerr << "Error: Supplied bad length register '"<<source[i].param[3] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}

		// Switch Register Mode
		if (source[i].param[0] == "mode"){
			if (source[i].param.size() < 3){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::mode;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(2);

			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Invalid target register "<<source[i].param[1] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			if       (source[i].param[2] == "uint8"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint8);
			}else if (source[i].param[2] == "int8"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int8);
			}else if (source[i].param[2] == "uint16"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint16);
			}else if (source[i].param[2] == "int16"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int16);
			}else if (source[i].param[2] == "float32"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::float32);
			}else if (source[i].param[2] == "uint32"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint32);
			}else if (source[i].param[2] == "int32"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int32);
			}else if (source[i].param[2] == "float64"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::float64);
			}else if (source[i].param[2] == "uint64"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint64);
			}else if (source[i].param[2] == "int64"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int64);
			}

			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Supplied bad target register '"<<source[i].param[1]<<"' (line: "<< source[i].line <<")" << std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}

		// Switch Register Mode, translating the data accordingly
		if (source[i].param[0] == "translate"){
			if (source[i].param.size() < 3){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::translate;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(2);

			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Invalid target register "<<source[i].param[1] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			if       (source[i].param[2] == "uint8"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint8);
			}else if (source[i].param[2] == "int8"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int8);
			}else if (source[i].param[2] == "uint16"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint16);
			}else if (source[i].param[2] == "int16"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int16);
			}else if (source[i].param[2] == "float32"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::float32);
			}else if (source[i].param[2] == "uint32"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint32);
			}else if (source[i].param[2] == "int32"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int32);
			}else if (source[i].param[2] == "float64"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::float64);
			}else if (source[i].param[2] == "uint64"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint64);
			}else if (source[i].param[2] == "int64"){
				this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int64);
			}

			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Supplied bad target register '"<<source[i].param[1]<<"' (line: "<< source[i].line <<")" << std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}

		// Math operations
		if (source[i].param[0] == "math"){
			if (source[i].param.size() < 5){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::math;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(5);

			// A value
			if (source[i].param[1][0] == 'r'){
				this->code[ptr].param[0] = 1;
				this->code[ptr].param[1] = GetRegisterID(source[i].param[1]);

				if (this->code[ptr].param[0] == -1){
					std::cerr << "Error: Invalid A register " << source[i].param[1] << std::endl;
					std::cerr << "  line: " << source[i].line << std::endl;
					error = true;
					continue;
				}
			}else{
				this->code[ptr].param[0] = 0;
				this->code[ptr].param[1] = Hexidecimal::convert(source[i].param[1]);       // Constant value
			}

			// B value
			if (source[i].param[3][0] == 'r'){
				this->code[ptr].param[2] = 1;
				this->code[ptr].param[3] = GetRegisterID(source[i].param[3]);

				if (this->code[ptr].param[0] == -1){
					std::cerr << "Error: Invalid B register " << source[i].param[3] << std::endl;
					std::cerr << "  line: " << source[i].line << std::endl;
					error = true;
					continue;
				}
			}else{
				this->code[ptr].param[2] = 0;
				this->code[ptr].param[3] = Hexidecimal::convert(source[i].param[3]);       // Constant value
			}

			// Result register
			this->code[ptr].param[4] = GetRegisterID(source[i].param[4]);
			if (this->code[ptr].param[4] == -1){
				std::cerr << "Error: Invalid result register" << source[i].param[4] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			if (source[i].param[2] == "add"){
				this->code[ptr].param[1] = MathOpperation::add;
			}else if (source[i].param[5] == "subtract"){
				this->code[ptr].param[1] = MathOpperation::subtract;
			}else if (source[i].param[5] == "multiply"){
				this->code[ptr].param[1] = MathOpperation::multiply;
			}else if (source[i].param[5] == "divide"){
				this->code[ptr].param[1] = MathOpperation::divide;
			}else if (source[i].param[5] == "modulus"){
				this->code[ptr].param[1] = MathOpperation::modulus;
			}else if (source[i].param[5] == "exponent"){
				this->code[ptr].param[1] = MathOpperation::exponent;
			}else{
				std::cerr << "Error: Invalid math operation \""<< source[i].param[2] << "\"" <<std::endl;
				std::cerr << "  Line: "<< source[i].line <<std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}

		// Duplicate the properties of one register to another
		if (source[i].param[0] == "copy"){
			if (source[i].param.size() < 3){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::copy;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(2);
			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			this->code[ptr].param[1] = GetRegisterID(source[i].param[2]);

			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Invalid from register " << source[i].param[1] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}
			if (this->code[ptr].param[1] == -1){
				std::cerr << "Error: Invalid to register" << source[i].param[2] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}

		// Duplicate data from one position to another
		if (source[i].param[0] == "move"){
			if (source[i].param.size() < 4){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::move;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(3);
			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			this->code[ptr].param[1] = GetRegisterID(source[i].param[2]);
			this->code[ptr].param[2] = GetRegisterID(source[i].param[3]);

			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Invalid from register " << source[i].param[1] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}
			if (this->code[ptr].param[1] == -1){
				std::cerr << "Error: Invalid to register " << source[i].param[2] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}
			if (this->code[ptr].param[2] == -1){
				std::cerr << "Error: Invalid length register " << source[i].param[3] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}


			ptr++;
			continue;
		}

		// Compare two register's numetric value
		if (source[i].param[0] == "compare"){
			if (source[i].param.size() < 5){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::compare;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(4);
			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Invalid A register " << source[i].param[1] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			if       (source[i].param[2] == "="){
				this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::equal);
			}else if (source[i].param[2] == ">"){
				this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::greater);
			}else if (source[i].param[2] == "<"){
				this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::less);
			}else{
				std::cerr << "Error: Invalid comparason operation \""<< source[i].param[2] << "\"" <<std::endl;
				std::cerr << "  Line: "<< source[i].line <<std::endl;
				error = true;
				continue;
			}

			this->code[ptr].param[2] = GetRegisterID(source[i].param[3]);
			this->code[ptr].param[3] = GetRegisterID(source[i].param[4]);

			if (this->code[ptr].param[2] == -1){
				std::cerr << "Error: Invalid B register " << source[i].param[3] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}
			if (this->code[ptr].param[3] == -1){
				std::cerr << "Error: Invalid result register " << source[i].param[4] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}

		// Compare two chunks of bytes
		if (source[i].param[0] == "lcompare"){
			if (source[i].param.size() < 5){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< this->code[ptr].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::compare;
			this->code[ptr].line = (source[i].line);
			this->code[ptr].param.reserve(6);
			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Invalid A register " << source[i].param[1] << std::endl;
				std::cerr << "  line: " << this->code[ptr].line << std::endl;
				error = true;
				continue;
			}

			if       (source[i].param[2] == "="){
				this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::equal);
			}else if (source[i].param[2] == ">"){
				this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::greater);
			}else if (source[i].param[2] == "<"){
				this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::less);
			}else{
				std::cerr << "Error: Invalid comparason operation \""<< source[i].param[2] << "\"" <<std::endl;
				std::cerr << "  line: "<< this->code[ptr].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].param[2] = GetRegisterID(source[i].param[3]);
			this->code[ptr].param[3] = GetRegisterID(source[i].param[4]);

			if (this->code[ptr].param[2] == -1){
				std::cerr << "Error: Invalid B register " << source[i].param[3] << std::endl;
				std::cerr << "  line: " << this->code[ptr].line << std::endl;
				error = true;
				continue;
			}
			if (this->code[ptr].param[3] == -1){
				std::cerr << "Error: Invalid result register " << source[i].param[4] << std::endl;
				std::cerr << "  line: " << this->code[ptr].line << std::endl;
				error = true;
				continue;
			}


			if (source[i].param[5] == "little"){
				this->code[ptr].param[4] = 0;
			}else if (source[i].param[5] == "big"){
				this->code[ptr].param[4] = 1;
			}else{
				std::cerr << "Error: Invalid edianess; " << source[i].param[5] << std::endl;
				std::cerr << "  line: " << this->code[ptr].line << std::endl;
				error = true;
				continue;
			}


			this->code[ptr].param[5] = GetRegisterID(source[i].param[4]);
			if (this->code[ptr].param[5] == -1){
				std::cerr << "Error: Invalid length register " << source[i].param[5] << std::endl;
				std::cerr << "  line: " << this->code[ptr].line << std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}

		// Bitwise operations
		if (source[i].param[0] == "bit"){
			if (source[i].param.size() < 5){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::bitwise;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(5);

			// A
			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Invalid A register " << source[i].param[1] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			// Operation
			if       (source[i].param[1] == "and"){
				this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::AND);
			}else if (source[i].param[1] == "or"){
				this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::OR);
			}else if (source[i].param[1] == "xor"){
				this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::XOR);
			}else if (source[i].param[1] == "lshift"){
				this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::LeftShift);
			}else if (source[i].param[1] == "rshift"){
				this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::RightShift);
			}else if (source[i].param[1] == "not"){
				this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::NOT);
			}
			// B
			if (source[i].param[2][0] == 'r'){
				this->code[ptr].param[2] = 1;
				this->code[ptr].param[3] = GetRegisterID(source[i].param[3]);

				if (this->code[ptr].param[0] == -1){
					std::cerr << "Error: Invalid B register " << source[i].param[3] << std::endl;
					std::cerr << "  line: " << source[i].line << std::endl;
					error = true;
					continue;
				}
			}else{
				this->code[ptr].param[2] = 0;
				this->code[ptr].param[3] = Hexidecimal::convert(source[i].param[3]);       // Constant value
			}

			// Result Store
			this->code[ptr].param[4] = GetRegisterID(source[i].param[4]);
			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Invalid result register " << source[i].param[4] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			ptr++;
			continue;
		}


		// IF statement
		if (source[i].param[0] == "if"){
			if (source[i].param.size() < 2){
				std::cerr << "Error: Not enough arguments supplied" << std::endl;
				std::cerr << "  line: "<< source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::IF;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(2);
			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			// param[1] = jump else statement

			if (this->code[ptr].param[0] == -1){
				std::cerr << "Error: Invalid boolean register " << source[i].param[1] << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
			}

			depth++;
			ptr++;
			continue;
		}
		if (source[i].param[0] == "else"){
			this->code[ptr].command = Commands::ELSE;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(2);

			ptr++;
			continue;
		}
		if (source[i].param[0] == "end"){
			this->code[ptr].command = Commands::END;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(0);

			depth--;
			ptr++;
			continue;
		}


		// Flow Navigation
		if (source[i].param[0] == "break"){
			this->code[ptr].command = Commands::Break;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(1);

			if (source[i].param.size() > 1){
				this->code[ptr].param[0] = Hexidecimal::convert(source[i].param[1]);
			}else{
				this->code[ptr].param[0] = 0;
			}

			ptr++;
			continue;
		}
		if (source[i].param[0] == "continue"){
			this->code[ptr].command = Commands::Continue;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(1);
			// param[0] = distance to loop beginning (command num)

			if (source[i].param.size() > 1){
				this->code[ptr].param[0] = Hexidecimal::convert(source[i].param[1]);
			}else{
				this->code[ptr].param[0] = 0;
			}

			ptr++;
			continue;
		}


		// Loop statement
		if (source[i].param[0] == "loop"){
			this->code[ptr].command = Commands::Loop;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.reserve(1);
			// param[0] = distance to loop end (command num)

			depth++;
			ptr++;
			continue;
		}


		// Break out sub function
		if (source[i].param[0] == "function"){
			found = false;
			d2 = 1;

			for (unsigned long j=i+1; j<codeLen; j++){

				// Ignore empty lines
				if (source[j].param.size() < 1){
					continue;
				}
				if (source[j].param[0] == "if"){
					d2 += 1;
					continue;
				}
				if (source[j].param[0] == "loop"){
					d2 += 1;
					continue;
				}
				if (source[j].param[0] == "function"){
					d2 += 1;
					continue;
				}
				if (source[j].param[0] == "end"){
					d2 -= 1;

					if (d2 == 0){
						found = true;

						// Forward the new code for a child function
						//                   |           source.slice(i, j)               |
						if (this->Subdivide( Segregate::StrCommands(&source[i], &source[j]) ) == false){
							error = true;
						}
						i = j;

						break;
					}

					continue;
				}
			}

			if (!found){
				error = true;
				std::cerr << "Error: Unable to find end of function" << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
			}
			continue;
		}


		// Spawn function instances
		if (source[i].param[0] == "instance"){
			this->code[ptr].command = Commands::instance;
			this->code[ptr].line = source[i].line;

			if (source[i].param.size() < 2){
				std::cerr << "Error: Undefined instance behaviour"<<std::endl;
				std::cerr << "  line: " << source[i].line;
				error = true;
				continue;
			}

			if (source[i].param[1] == "create"){
				if (source[i].param.size() < 4){
					std::cerr << "Error: Not enough arguments to generate an instance"<<std::endl;
					std::cerr << "  line: " << source[i].line;
					error = true;
					continue;
				}

				this->code[ptr].param.resize(3);
				this->code[ptr].param[0] = 0;
				this->code[ptr].param[1] = GetChildsID(source[i].param[2]);
				this->code[ptr].param[2] = GetRegisterID(source[i].param[3]);

				// Check function name validity
				if (this->code[ptr].param[1] == -1){
					std::cerr << "Error: Attempting to create instance of undefined function \"" << source[i].param[2] << "\"" << std::endl;
				}

				// Check register validity
				if (this->code[ptr].param[2] == -1){
					std::cerr << "Error: Supplied bad instance reference register "<<source[i].param[1] << std::endl;
					std::cerr << "  line: " << source[i].line << std::endl;
					error = true;
					continue;
				}

				ptr++;
				continue;
			}

			if (source[i].param[1] == "yeild"){
				this->code[ptr].param.resize(3);
				this->code[ptr].param[0] = 1;
				this->code[ptr].param[1] = GetRegisterID(source[i].param[2]);
				// this->code[ptr].param[2] = read over length

				// Check register validity
				if (this->code[ptr].param[1] == -1){
					std::cerr << "Error: Supplied bad instance reference register "<<source[i].param[1] << std::endl;
					std::cerr << "  line: " << source[i].line << std::endl;
					error = true;
					continue;
				}

				ptr++;
				continue;
			}

			if (source[i].param[1] == "return"){
				this->code[ptr].param.resize(1);
				this->code[ptr].param[0] = 2;
				this->code[ptr].param[1] = GetRegisterID(source[i].param[2]);
				// this->code[ptr].param[2] = read over length

				// Check register validity
				if (this->code[ptr].param[1] == -1){
					std::cerr << "Error: Supplied bad instance reference register "<<source[i].param[1] << std::endl;
					std::cerr << "  line: " << source[i].line << std::endl;
					error = true;
					continue;
				}

				ptr++;
				continue;
			}

			if (source[i].param[1] == "execute"){
				this->code[ptr].param.resize(3);
				this->code[ptr].param[0] = 3;
				this->code[ptr].param[1] = GetRegisterID(source[i].param[2]);
				this->code[ptr].param[2] = 0;

				// Check register validity
				if (this->code[ptr].param[1] == -1){
					std::cerr << "Error: Supplied bad instance reference register "<<source[i].param[1] << std::endl;
					std::cerr << "  line: " << source[i].line << std::endl;
					error = true;
					continue;
				}

				if (source[i].param.size() > 3){
					if (source[i].param[3] == "main"){
						this->code[ptr].param[2] = 1;
					}else{
						std::cerr << "Error: Invalid extra execution argument \"" << source[i].param[3] << '"' << std::endl;
						std::cerr << "  line: " << source[i].line << std::endl;
						error = true;
						continue;
					}
				}

				ptr++;
				continue;
			}


			std::cerr << "Error: Invalid instance action \"" << source[i].param[1] << '"' << std::endl;
			std::cerr << "  line: " << source[i].line;
			error = true;
			continue;
		}


		if (source[i].param[0] == "local"){
			if (source[i].param.size() < 2){
				std::cerr << "Error: Not enought arguments" << std::endl;
				std::cerr << "  line: " << source[i].line << std::endl;
				error = true;
				continue;
			}

			this->code[ptr].command = Commands::local;
			this->code[ptr].line = source[i].line;
			this->code[ptr].param.resize(2);
			this->code[ptr].param[0] = GetRegisterID(source[i].param[1]);
			this->code[ptr].param[1] = 0;

			if (this->code[ptr].param[2] == -1){
				std::cerr << "Error: Invalid result register \"" << source[i].param[1] << '"' << std::endl;
				std::cerr << "  line: " << source[i].line;
				error = true;
				continue;
			}


			if (source[i].param.size() > 2){
				this->code[ptr].param.resize(3);

				this->code[ptr].param[2] = GetRegisterID(source[i].param[2]);
				if (this->code[ptr].param[2] == -1){
					std::cerr << "Error: Invalid instance pointer register \"" << source[i].param[1] << '"' << std::endl;
					std::cerr << "  line: " << source[i].line;
					error = true;
					continue;
				}
			}

			ptr++;
			continue;
		}


		std::cerr << "Error: Invalid Command '" << source[i].param[0] << "'"<<std::endl;
		std::cerr << "  line: "<<source[i].line<<std::endl;
		error = true;
	}


	// Shrink the vector to the correct size
	this->code.resize(ptr);  // Shrink the length of the vector
	this->code.reserve(ptr); // Re-allocate to save space

	if (error){
		return false;
	}

	return true;
};

bool Function::SimplifyIF(){
	unsigned int elseLoc;  // Where the Else clause starts
	bool hasElse;          // Does the statement have an else clause
	bool hasEnd;

	unsigned int depth = 0; // Ignore inner IF content while scanning for else/endif

	unsigned int length = this->code.size();
	unsigned int i;
	unsigned int j;
	for (i=0; i<length; i++){

		if (this->code[i].command == Commands::IF){
			depth = 1;
			hasElse = false;
			hasEnd = false;

			// Find the extra statements
			for (j=i+1; j<length; j++){
				if (
					this->code[j].command == Commands::IF
					// Loop already simplied and removed 'end' commands
				){
					depth += 1;
					continue;
				}

				if (this->code[j].command == Commands::ELSE){
					elseLoc = j;
					hasElse = true;
				}

				if (this->code[j].command == Commands::END){
					depth -= 1;

					if (depth == 0){
						hasEnd = true;
						break;
					}
				}
			}

			if (hasEnd == false){
				return false;
			}

			// How far to skip on the false statement
			this->code[i].param.reserve(1);
			if (hasElse){
				// Has an else statement,

				// So ensure that the true statement will not overflow in to the else
				this->code[elseLoc].command = Commands::GOTO;
				this->code[elseLoc].param.reserve(1);
				this->code[elseLoc].param[0] = j;

				// Give the if statement a jump to point for the false condition
				this->code[i].param[1] = elseLoc;
			}else{
				// No else statement, so just skip to the end
				this->code[i].param[1] = j;
			}
			this->code[j].command = Commands::blank;
			this->code[j].param.reserve(0);
		}

	}

	return true;
};

bool Function::SimplifyLoop(){
	unsigned long length = this->code.size();
	unsigned long depth = 0;
	bool found = false;

	// Specify loops with their lengths
	// Make loop endpoints wrap
	for (unsigned long i=0; i<length; i++){
		if (this->code[i].command == Commands::Loop){
			depth = 1;
			found = false;

			for (unsigned long j=i+1; j<length; j++){

				// Ensure depth
				if (
					this->code[j].command == Commands::Loop ||
					this->code[j].command == Commands::IF
				){
					depth += 1;
					continue;
				}

				if (this->code[j].command == Commands::END){
					depth -= 1;

					if (depth == 0){
						this->code[i].param[0] = j-i;

						// Change the end mark of the while loop
						// To a backwards jump to continue the loop
						this->code[j].command = Commands::GOTO;
						this->code[j].param.reserve(1);
						this->code[j].param[0] = i;

						// Assign loop length (for breaks)
						this->code[i].param[0] = j+1;

						found = true;

						break;
					}
				}
			}

			if (!found){
				std::cerr << "Error: Missing loop endpoint"<<std::endl;
				std::cerr << "  line: "<<this->code[i].line<<std::endl;
				return false;
			}
		}
	}

	// Bind continue command
	for (unsigned long i=0; i<length; i++){
		if (this->code[i].command == Commands::Continue){
			depth = this->code[i].param[0] + 1;
			found = false;

			// Find the n-th previous loop command
			for (long j=i-1; j>=0; j--){

				// Change of depth
				if(
					this->code[j].command == Commands::Loop  &&
					this->code[j].param[0] >= i-j                // Loop covers the target
				){
					depth -= 1;
				}

				if (depth == 0){
					this->code[i].command = Commands::GOTO;
					this->code[i].param.reserve(1);
					this->code[i].param[0] = j;
					found = true;

					break;
				}
			}

			if (!found){
				std::cerr << "Error: Failed to find loop with correct depth for continue statement (" << this->code[i].param[0] << ")"<<std::endl;
				std::cerr << "  line: "<<this->code[i].line<<std::endl;
				return false;
			}
		}
	}

	// Bind break command
	for (unsigned long i=0; i<length; i++){
		if (this->code[i].command == Commands::Break){
			depth = this->code[i].param[0] + 1;
			found = false;

			// Find the n-th previous loop command
			for (long j=i-1; j>=0; j--){

				// Change of depth
				if(
					this->code[j].command == Commands::Loop  &&
					this->code[j].param[0] >= i-j                // Loop covers the target
				){
					depth -= 1;
				}

				if (depth == 0){

					this->code[i].command = Commands::GOTO;
					this->code[i].param.reserve(1);
					this->code[i].param[0] = this->code[j].param[0];
					found = true;

					break;
				}
			}

			if (!found){
				std::cerr << "Error: Failed to find loop with correct depth for break statement (" << this->code[i].param[0] << ")"<<std::endl;
				std::cerr << "  line: "<<this->code[i].line<<std::endl;
				return false;
			}
		}
	}


	return true;
};

bool Function::SimplifyInstance(){
	unsigned long length = this->code.size();
	bool found = false;
	long depth = 0;
	bool error = false;

	// Remember if statements and loops have already been resolved

	// Change instance yeilds
	for (unsigned long i=0; i<length; i++){
		if (
			this->code[i].command == Commands::instance &&
			( this->code[i].param[0] == 1 || this->code[i].param[0] == 2 ) // yeild/return
		){
			found = false;
			depth = 1;

			for (unsigned long j=i+1; j<length; j++){
				if (
					this->code[j].command == Commands::instance &&
					( this->code[j].param[0] == 1 || this->code[j].param[0] == 2 ) // yeild/return
				){
					depth += 1;
					continue;
				}

				if (this->code[j].command == Commands::END){
					depth -= 1;
				}

				if (depth == 0){
					this->code[i].param[2] = j;

					// Erase the end point
					this->code[j].command = Commands::stop; // On return, do not over extend
					this->code[j].param.reserve(0);

					found = true;
					break;
				}
			}

			if (!found){
				std::cerr << "Error: Unable to find end" << std::endl;
				std::cerr << "  line: " << this->code[i].line << std::endl;
				error = true;
				continue;
			}
		}
	}

	return !error;
}

bool Function::CheckBlocks(){
	unsigned long length = this->code.size();
	bool error = false;

	for (unsigned long i=0; i<length; i++){
		if (this->code[i].command == Commands::END){
			std::cerr << "Error: Unexpected block closure" << std::endl;
			std::cerr << "  line: "<<this->code[i].line<<std::endl;
			error = true;
		}
	}

	return !error;
}


bool Function::Parse(Segregate::StrCommands src){
	if (this->Interpret(src) == false){
		return false;
	}
	if (this->SimplifyLoop() == false){
		return false;
	}
	if (this->SimplifyIF() == false){
		return false;
	}
	if (this->SimplifyInstance() == false){
		return false;
	}
	if (this->CheckBlocks() == false){
		return false;
	}

	return true;
};
