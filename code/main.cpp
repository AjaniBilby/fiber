/*
  Compile the project from this file to have a standalone fabric execute of which functions within the command line.
*/

#include <iostream>
#include <fstream>
#include <vector>

#include "./lib/thread.h"

#define FIBER_VERSION "v0.0.1a"
bool FLAG_TIME = false;






int main(int argc, char* argv[]){
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
    std::cout << "Invalid file: " << argv[1] << std::endl;
    return 1;
  }





  Thread::Init();

  // Mount the data into a function tree
  Function entry ("_root_",0,0);
  if (entry.Parse( Segregate::Fragment(fileData) ) == false){
    std::cerr << "Error: Failed to interpret" << std::endl;
  }else{
    // Start execution
    Instance root(&entry, nullptr);
    Thread::Dispatch(&root, 0);

    Thread::Wedge();

    std::cout << std::endl;
  }

  return 0;
}