/*
	Compile the project from this file to have a standalone fabric execute of which functions within the command line.
*/

#include <iostream>
#include <fstream>
#include <vector>

#include "./lib/thread.hpp"

#define FIBER_VERSION "v0.0.1a"
bool FLAG_TIME = false;






int main(int argc, char* argv[]){
	unsigned int threads = 0;

	if (ValidTypeSizing() == false){
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

		// -time
		if (argv[i][0] == '-' && argv[i][1] == 't' && argv[i][2] == 'i' && argv[i][3] == 'm' && argv[i][4] == 'e'){
			FLAG_TIME = true;
		}

		// -thread
		if (argv[i][0] == '-' && argv[i][1] == 't' && argv[i][2] == 'h' && argv[i][3] == 'r' && argv[i][4] == 'e' && argv[i][5] == 'a' && argv[i][6] == 'd'){
			if (argc <= i+1){
				std::cout << "Missing thread flag argument" << std::endl;
				return 0;
			}

			threads = std::atoi(argv[i+1]);
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
		std::cout << "Invalid file: " << argv[1] << std::endl;
		return 1;
	}






	// Mount the data into a function tree
	Function entry ("_root_",0,0);
	if ( entry.Parse( Segregate::Fragment(std::string(fileData)) ) == false ){
		std::cerr << "Error: Failed to interpret" << std::endl;
		return 0;
	}
	if ( entry.PostProcess() == false ){
		std::cerr << "Error: Failed to process interpretation" << std::endl;
		return 0;
	}

	Thread::Pool pool(threads);

	// Start execution
	Instance root(&entry, nullptr, &pool, nullptr);
	Thread::Job first;
	first.ptr = &root;
	first.cursor = 0;
	pool.Dispatch(first, false, 0);

	pool.Wedge();
	std::cout << std::endl;

	return 0;
}
