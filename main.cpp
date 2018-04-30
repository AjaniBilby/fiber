#include "./lib/thread.cpp"
#include <vector>

std::vector<unsigned char> code;
unsigned int codeLength;

int main(){
  Thread::Initilize();
  Thread::KeepAlive();

  return 0;
}