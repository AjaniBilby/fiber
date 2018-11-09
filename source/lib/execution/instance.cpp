#include "./instance.hpp"


void Instance::Process(size_t pos){
	std::string str = "Thread " + std::to_string(this->owner->workerID) + " processing...\n";
	std::cout << str;

	return;
};
