#include "./bytecode.hpp"


uint64 Order::get(size_t i){
	if (i > this->params){
		return -1;
	}

	char *ptr = reinterpret_cast<char*>(this);
	ptr += sizeof(Order);
	ptr += sizeof(uint64) * i;

	return *( reinterpret_cast<uint64*>(ptr) );
};
uint64* Order::ref(size_t i){
	if (i > this->params){
		return nullptr;
	}

	char *ptr = reinterpret_cast<char*>(this);
	ptr += sizeof(Order);
	ptr += sizeof(uint64) * i;

	return reinterpret_cast<uint64*>(ptr);
};










Bytecode::~Bytecode(){
	// Unallocate each order
	Order* curr = this->next();
	Order* nxt;

	while (curr != nullptr){
		nxt = this->next(curr);

		delete curr;

		curr = nxt;
	}
};

void Bytecode::append(Action act){
	size_t params = act.param.size();
	size_t bytes  = sizeof(Order) + params*sizeof(act.param[0]);

	// Allocate order with excess space to store parameters as trailing data
	Order *addr = (Order*)malloc(bytes);

	// Write metadata
	addr->cmd    = act.cmd;   // Command ID
	addr->line   = act.line;  // Line number
	addr->params = params;    // Number of parameters
	addr->next   = nullptr;   // Mark this as the end of the chain

	// Transfer the parameter values
	uint64* ptr;
	for (size_t i=0; i<params; i++){
		ptr = addr->ref(i);

		if (ptr == nullptr){
			std::cerr << "An unknown error has occured. Action parameter count changed mid writting to bytecode" << std::endl;
			std::cerr << "  command: " << ToString(act) << std::endl;
		}

		*ptr = act.param[i];
	}

	// Chain the new order to the linked list
	if (this->last != nullptr){
		this->last->next = addr;
	}
	this->last = addr;

	// If the chain does not have a starting order, make this new order the start
	if (this->first == nullptr){
		this->first = addr;
	}

	return;
};

Order* Bytecode::next(Order* current){
	if (current == nullptr){
		return this->first;
	}

	return current->next;
};

Order* Bytecode::at(size_t i){
	Order* ptr = this->first;

	// Iterate until on the ith element or reached the end (marked by null ptr)
	while (ptr != nullptr && i > 0){
		ptr = ptr->next;
		i--;
	}

	return ptr;
};

Order* Bytecode::getLast(){
	return this->last;
}


bool Bytecode::simplify(){
	if (this->simplifyJumps() == false){
		return false;
	}

	this->simplifyRemoveBlanks();

	return true;
};
bool Bytecode::simplifyJumps(){
	Order* ptr = this->next();
	void* goal = nullptr;
	int64* data;
	while (ptr != nullptr){
		if (ptr->cmd == Command::jump){
			// Get a reference to the jump's first parameter
			data = reinterpret_cast<int64*>( ptr->ref(0) );

			// Find the exact address of the goal position
			goal = this->at( *data );
			if (goal == nullptr){
				std::cerr << "Error: Invalid jump amount " << data << "." << std::endl;
				std::cerr << "  line: " << ptr->line << std::endl;
				return false;
			}

			// Write this address to the first parameter
			*data = reinterpret_cast<uint64>(goal);
		}

		ptr = this->next(ptr);
	}

	return true;
};
bool Bytecode::simplifyRemoveBlanks(){
	// NOTE: MUST EXECUTE AFTER SIMPLIFY JUMPS

	Order* prev = nullptr;
	Order* curr = this->next();

	while (curr != nullptr){
		if (curr->cmd == Command::blank){
			// If this is the first element
			if (prev == nullptr){
				this->first = this->next(curr); // Remove the element from the start of the chain
				free(curr);                     // Unallocate the unlinked element

				// Increment
				curr = this->first;
				continue;
			}

			prev->next = this->next(curr);
			free(curr);
			curr = prev->next;

			continue;
		}

		prev = curr;
		curr = this->next(curr);
	}

	this->last = prev;

	return true;
}










std::string ToString(Action code){
	unsigned long size = code.param.size();

	// Get the CommandID as a string
	std::string str = ToString(code.cmd);
	str += "(" + std::to_string(code.line) + ", " + std::to_string(size) + ")";

	for (size_t i=0; i<size; i++){
		str += " " + std::to_string(code.param[i]);
	}

	return str;
};

std::string ToString(Order *ptr){
	if (ptr == nullptr){
		return "Err(Null pointer)";
	}

	std::string str = ToString(ptr->cmd);
	str += "(" + std::to_string(ptr->line) + ", " + std::to_string(ptr->params) + ")";

	for (size_t i=0; i<ptr->params; i++){
		str += " " + std::to_string(ptr->get(i));
	}

	return str;
};
