#include "./flags.hpp"

#include "./lib/interprete/tokenize.hpp"
#include "./lib/interprete/function.hpp"

#include "./lib/execution/thread.hpp"

#include <thread>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>





int main(int argc, char* argv[]){
	if (CheckTypeSizing() == false){
		std::cerr << std::endl << "Error: Sizing error obstructing execution" << std::endl;
		return 1;
	}


	size_t hardwareThreads = std::thread::hardware_concurrency();
	if (hardwareThreads != 0){
		THREAD_COUNT = hardwareThreads;
	}


	if (argc < 2){
		std::cerr << "Missing file input" << std::endl;
		return 0;
	}

	// Read flags
	for (size_t i=1; i<argc; i++){

		// -thread X
		if (
			argv[i][0] == '-' &&
			argv[i][1] == 't' &&
			argv[i][2] == 'h' &&
			argv[i][3] == 'r' &&
			argv[i][4] == 'e' &&
			argv[i][5] == 'a' &&
			argv[i][6] == 'd'
		){
			if (argc <= i+1){
				std::cerr << "Error: Missing thread count parameter" << std::endl;
				return 1;
			}

			THREAD_COUNT = std::stoull(argv[i+1]);
			i++; // consumed next parameter

			std::cout << "Threads: " << THREAD_COUNT << std::endl;

			continue;
		}

		// -version
		if (
			argv[i][0] == '-' &&
			argv[i][1] == 'v' &&
			argv[i][2] == 'e' &&
			argv[i][3] == 'r' &&
			argv[i][4] == 's' &&
			argv[i][5] == 'i' &&
			argv[i][6] == 'o' &&
			argv[i][7] == 'n'
		){
			std::cout << FIBER_VERSION << std::endl;
			continue;
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

	// Interprete data
	auto tokens = Tokenize::SplitLines(fileData);
	fileData.resize(0); // Delete the original file cache
	std::cout << "Tokenized" << std::endl;
	Function root = Function("root", tokens, 0, nullptr);
	// tokens.resize(0);

	if (root.valid == false){
		std::cerr << std::endl << "Error: Unable to execute due to invalid code." << std::endl;
		return 1;
	}

	std::cout << "Interpreted code" << std::endl;
	return 0;


	// Start up execution space
	Thread::Pool workSpace(THREAD_COUNT);

	// Create dumny instances to check the event loops are working
	Instance dumy(&root);
	workSpace.Issue({&dumy, 0});

	// Blocks until all workers have no remaining work
	workSpace.WaitUntilDone();
	std::cout << "All work completed" << std::endl;

	// Clean up all threads
	workSpace.Close();
	std::cout << "Closed" << std::endl;

	return 0;
}
