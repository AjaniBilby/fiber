#include "./lib/thread.cpp"
#include <iostream>
#include <fstream>
#include <vector>

bool FLAG_TIME = false;

int main(int argc, char* argv[]){
  if (argc < 2){
    std::cout << "Missing file input" << std::endl;
    return 0;
  }

  // Version
  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'v'){
    std::cout << "v0.0.0" << std::endl;
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
  std::streampos fileSize;
  std::vector<char> fileData;

  if (file.is_open()){

    // Get file size
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read file
    fileData.reserve(fileSize);
    file.read((char*) &fileData[0], fileSize);
  }else{
    std::cout << "Invalid path: " << argv[1];
    return 1;
  }


  std::cout << std::endl;
  std::cout << std::endl;


  Thread::Initilize();
  float duration = Thread::KeepAlive(); // Returns the execution time
  if (FLAG_TIME){
    std::cout << "-----------------------------------------------" << std::endl;
    std::cout << "  Assemble Time: " << 0 << " sec" << std::endl;
    std::cout << "  Execution Time: " << duration << " sec" << std::endl;
    std::cout << "  Total: " << 0 + duration << std::endl;
  }

  return 0;
}