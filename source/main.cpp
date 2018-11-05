#include "./flags.hpp"

#include "./lib/interprete/tokenize.hpp"
#include "./lib/interprete/function.hpp"

#include "./lib/execution/thread.hpp"

#include <iostream>
#include <fstream>
#include <vector>





int main(int argc, char* argv[]){
	if (checkTypeSizing() == false){
		std::cerr << std::endl << "Error: Sizing error obstructing execution" << std::endl;
		return 1;
	}


	if (argc < 2){
		std::cout << "Missing file input" << std::endl;
		return 0;
	}

	// Version
	if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'v'){
		std::cout << FIBER_VERSION << std::endl;
		return 0;
	}

	// Read flags
	for (int i=0; i<argc; i++){
		if (argv[i][0] == '-'){
			if (argv[i][1] == 't'){
				FLAG_TIME = true;
			}
		}
	}





	// Read the file
	std::ifstream file(argv[1], std::ios::binary);
	std::string fileData;
	if (file.is_open()){
		file.seekg(0, std::ios::end);
		fileData.reserve(file.tellg());
		file.seekg(0, std::ios::beg);

		fileData.assign((std::istreambuf_iterator<char>(file)),
										 std::istreambuf_iterator<char>());
	}else{
		std::cerr << "Invalid file: " << argv[1] << std::endl;
		return 1;
	}

	auto tokens = Tokenize::SplitLines(fileData);

	std::cout << "Tokens;" << std::endl;
	unsigned long size = tokens.size();
	for (unsigned long i=0; i<size; i++){
		std::cout << tokens[i].line << ": " << ToString(tokens[i].param) << std::endl;
	}
	std::cout << std::endl;

	Function root = Function("root", tokens, 0, nullptr);
	fileData.resize(0); // Delete the original file cache

	if (root.valid == false){
		std::cerr << "Unable to execute due to error(s)" << std::endl;
		return 1;
	}

	return 0;
}
