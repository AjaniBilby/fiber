#include "interpreter.hpp"

std::string ToString(std::vector<std::string> arr){
	unsigned long size = arr.size();
	std::string str;
	
	for (unsigned long i=0; i<size; i++){
		if (i != 0){
			str += ' ';
		}
		
		str += arr[i];
	}
	
	return str;
};


namespace Interpreter{
	Opperand::Opperand(){
		this->valid = false;
	}
	Opperand::Opperand(std::string str){
		// Default values
		this->type = OpperandType::Unknown;
		this->valid = false;

		if (str[0] == '@'){
			this->type = OpperandType::RegisterAddress;
			this->data.int8 = GetRegisterID(str.substr(1, 2));
			this->valid = (this->data.int8 != -1);
			this->data.uint64 = this->data.int8;
			return;
		}
		if (str[0] == '&'){
			this->type = OpperandType::RegisterValue;
			this->data.int8 = GetRegisterID(str.substr(1, 2));
			this->valid = (this->data.int8 != -1);
			this->data.uint64 = this->data.int8;
			return;
		}

		uint64 length = str.size();

		if (length > 2){
			if (str[0] == '0' && str[1] == 'x'){
				this->type = OpperandType::Bytes;
				std::stringstream temp;
				temp << std::hex << str;
				temp >> this->data.uint64;
				this->valid = true;
				return;
			}
		}
		if (length > 1){
			if (str[length-1] == 'i'){
				this->type = OpperandType::Int;
				this->data.int64 = stoll(str.substr(0, length-1));
				this->valid = true;
				return;
			}
			if (str[length-1] == 'u'){
				this->type = OpperandType::Uint;
				this->data.uint64 = stoull(str.substr(0, length-1));
				this->valid = true;
				return;
			}
			if (str[length-1] == 'f'){
				this->type = OpperandType::Float;
				this->data.float64 = stod(str.substr(0, length-1));
				this->valid = true;
				return;
			}
		}
	};

	std::string Opperand::typeToString(){
		switch(this->type){
			case Interpreter::OpperandType::Uint:
				return "uint";
			case Interpreter::OpperandType::Bytes:
				return "bytes";
			case Interpreter::OpperandType::Float:
				return "float";
			case Interpreter::OpperandType::Int:
				return "int";
			case Interpreter::OpperandType::RegisterAddress:
				return "register address";
			case Interpreter::OpperandType::RegisterValue:
				return "register value";
			case Interpreter::OpperandType::Unknown:
				return "unknown";
		}
	};
};



namespace Interpreter{
	Action InterpSet(RawAction act){
		Action out;
		out.cmd = Command::set;
		out.line = act.line;
		
		if (act.param.size() != 4){
			std::cerr << "Error: Invalid number of arguments for set command" << std::endl;
			std::cerr << "  args: " << ToString(act.param) << std::endl;
			std::cerr << "  line: " << act.line << std::endl;
			
			out.cmd = Command::invalid;
			return out;
		}
		out.param.resize(5);
		
		
		// Interpret target
		auto opper = Interpreter::Opperand(act.param[1]);
		if (opper.type != Interpreter::OpperandType::RegisterAddress && opper.type != Interpreter::OpperandType::RegisterValue){
			std::cerr << "Error: Invalid target type of set command" << std::endl;
			std::cerr << "   arg: " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;
			out.cmd = Command::invalid;
			return out;
		}
		if (opper.valid == false){
			std::cerr << "Error: Invalid target register of set command" << std::endl;
			std::cerr << "   arg: " << act.param[1] <<std::endl;
			std::cerr << "  line: " << act.line << std::endl;
			out.cmd = Command::invalid;
			return out;
		}
		out.param[0] = static_cast<uint64>(opper.type);
		out.param[1] = opper.data.uint64;

		// Interpret value
		if       (act.param[3] == "local"){
			out.param[2] = 1;
			out.param[3] = 1;

			if (opper.type != Interpreter::OpperandType::RegisterAddress){
				std::cerr << "Error: Setting to 'local' requires the target to be a register address" << std::endl;
				std::cerr << "  line: " << act.line << std::endl;
				out.cmd = Command::invalid;
			return out;
			}
		}else if (act.param[3] == "parse"){
			out.param[2] = 1;
			out.param[3] = 2;

			if (opper.type != Interpreter::OpperandType::RegisterAddress){
				std::cerr << "Error: Setting to 'parse' requires the target to be a register address" << std::endl;
				std::cerr << "  line: " << act.line << std::endl;
				out.cmd = Command::invalid;
				return out;
			}
		}else{

			// Changing the value or address of a register?
			if (opper.type == Interpreter::OpperandType::RegisterAddress){
				// Cannot set the address of a register to a constant value
				opper = Interpreter::Opperand(act.param[3]);
				if (opper.type != Interpreter::OpperandType::RegisterAddress && opper.type != Interpreter::OpperandType::RegisterValue){
					std::cerr << "Error: Invalid value type of set command" << std::endl;
					std::cerr << "   arg: " << act.param[3] <<std::endl;
					std::cerr << "  line: " << act.line << std::endl;
					out.cmd = Command::invalid;
					return out;
				}
				if (opper.valid == false){
					std::cerr << "Error: Invalid value register of set command" << std::endl;
					std::cerr << "   arg: " << act.param[3] <<std::endl;
					std::cerr << "  line: " << act.line << std::endl;
					out.cmd = Command::invalid;
					return out;
				}
			}else{
				opper = Interpreter::Opperand(act.param[3]);
			}
			out.param[2] = 0;
			out.param[3] = static_cast<uint64>(opper.type);
			out.param[4] = opper.data.uint64;
		}
		
		
		return out;
	};
	Action InterpRtrn(RawAction act){
		Action out;
		out.cmd = Command::rtrn;
		out.line = act.line;
		
		if (act.param.size() != 1){
			std::cerr << "Error: Invalid number of arguments for return command" << std::endl;
			std::cerr << "  args: " << ToString(act.param) << std::endl;
			std::cerr << "  line: " << act.line << std::endl;
			
			out.cmd = Command::invalid;
			return out;
		}
		out.param.resize(0);
		
		
		return out;
	}
	
	
	
	Action Convert(RawAction act){
		Action out;
		out.cmd = CommandFrom(act.param[0]);
		
		switch (out.cmd){
			case Command::invalid:
				break;
			case Command::set:
				return InterpSet(act);
			case Command::rtrn:
				return InterpRtrn(act);
		}
		
		out.cmd = Command::invalid;
		return out;
	};
}
