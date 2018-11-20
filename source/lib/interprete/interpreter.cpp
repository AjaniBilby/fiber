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

		// Register Address opperand
		if (str[0] == '@'){
			this->type = OpperandType::RegisterAddress;
			this->data.int8 = GetRegisterID(str.substr(1, 2));
			this->valid = (this->data.int8 != -1);
			this->data.uint64 = this->data.int8;
			return;
		}
		// Register value opperand
		if (str[0] == '&'){
			this->type = OpperandType::RegisterValue;
			this->data.int8 = GetRegisterID(str.substr(1, 2));
			this->valid = (this->data.int8 != -1);
			this->data.uint64 = this->data.int8;
			return;
		}

		uint64 length = str.size();

		// Intake hexidecimal data as byte-wise data
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

		// Intake standard neumeric representations
		if (length > 1){
			// Int
			if (str[length-1] == 'i'){
				this->type = OpperandType::Int;
				this->data.int64 = stoll(str.substr(0, length-1));
				this->valid = true;
				return;
			}
			// Uint
			if (str[length-1] == 'u'){
				this->type = OpperandType::Uint;
				this->data.uint64 = stoull(str.substr(0, length-1));
				this->valid = true;
				return;
			}
			// Float (actually double)
			if (str[length-1] == 'f'){
				this->type = OpperandType::Float;
				this->data.float64 = stod(str.substr(0, length-1));
				this->valid = true;
				return;
			}
		}
	};

	// Convert a class' opperand type to string
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
	inline Action InterpBitwise(RawAction act){
		Action out;
		out.cmd = Command::bitwise;
		out.line = act.line;

		if (act.param.size() != 4){
			std::cerr << "Error: Invalid number of arguments for bitwise command" << std::endl;
			std::cerr << "  args: " << ToString(act.param) << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param.resize(4);


		// Interpret A
		auto opper = Interpreter::Opperand(act.param[1]);
		if (opper.type != Interpreter::OpperandType::RegisterValue){
			std::cerr << "Error: Invalid bitwise opperand A. Opperand must be Register Value type." << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		if (opper.valid == false){
			std::cerr << "Error: Invalid opperand A input." << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param[0] = opper.data.uint64;

		// Opperation
		if       (act.param[2] == "and"){
			out.param[1] = 0;
		}else if (act.param[2] == "or"){
			out.param[1] = 1;
		}else if (act.param[2] == "xor"){
			out.param[1] = 2;
		}else if (act.param[2] == "not"){
			out.param[1] = 3;
		}else if (act.param[2] == "shift"){
			out.param[1] = 4;
		}


		// Invalid Opperand B for NOT
		if (out.param[1] == 3 && act.param[3] != "_"){
			std::cerr << "Error: Invalid opperand B for bitwise not execution. Opperand B must be \"_\"." << std::endl;
			std::cerr << "  arg : " << act.param[3] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		opper = Interpreter::Opperand(act.param[3]);
		// Invalid Opperand B for Shift
		if (opper.type != Interpreter::OpperandType::Int && out.param[1] == 4){
			std::cerr << "Error: Invalid opperand B for bitwise shift execution. Opperand B must be a signed integer." << std::endl;
			std::cerr << "  arg : " << act.param[3] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		// Bitwise cannot execute on an address
		if (opper.type == Interpreter::OpperandType::RegisterAddress){
			std::cerr << "Error: Invalid opperand B for bitwise opperation. Opperands cannot be register addresses." << std::endl;
			std::cerr << "  arg : " << act.param[3] << std::endl;
			std::cerr << "  line: " << act.param[3] << std::endl;

			out.cmd = Command::invalid;
			return out;
		}


		return out;
	}

	inline Action InterpBlock(RawAction act){
		Action out;
		out.line = act.line;

		if (act.param.size() != 1){
			std::cerr << "Error: Invalid number of arguments for block open/close command" << std::endl;
			std::cerr << "  args: " << ToString(act.param) << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param.resize(0);

		if (act.param[0] == "{"){
			out.cmd = Command::blockOpen;
		}
		if (act.param[0] == "}"){
			out.cmd = Command::blockClose;
		}


		return out;
	}

	inline Action InterpCompare(RawAction act){
		Action out;
		out.cmd = Command::compare;
		out.line = act.line;

		// Check this line is actually valid and contains a command
		if (act.param.size() != 5){
			std::cerr << "Error: Invalid number of arguments for compare command" << std::endl;
			std::cerr << "  args: " << ToString(act.param) << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}

		bool addressMode = false;

		// Interp operand A
		//  Check the opperand type is valid
		auto opper = Interpreter::Opperand(act.param[1]);
		if (opper.type == Interpreter::OpperandType::RegisterAddress){     // Valid opperand type
			addressMode = true;
		}else if (opper.type == Interpreter::OpperandType::RegisterValue){ // Valid opperand type
		}else{                                                             // Invalid opperand type
			std::cerr << "Error: Invalid opperand A; must be a register value or address." << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}

		//  Check the data is valid
		if (opper.valid == false){
			std::cerr << "Error: Invalid opperand A register." << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << out.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param[0] = static_cast<uint64>(opper.type);
		out.param[1] = opper.data.uint64;


		// Opperator
		if (act.param[2] == "<"){
			out.param[0] = static_cast<uint64>(ComparisonOpperators::lesser);
		}else if (act.param[2] == ">"){
			out.param[0] = static_cast<uint64>(ComparisonOpperators::greater);
		}else if (act.param[2] == "="){
			out.param[0] = static_cast<uint64>(ComparisonOpperators::equal);
		}else{
			std::cerr << "Error: Invalid opperator for comparison" << std::endl;
			std::cerr << "  arg : " << act.param[2] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}


		// Interp operand B
		//  Check the data type
		opper = Interpreter::Opperand(act.param[3]);
		if (opper.type == Interpreter::OpperandType::RegisterAddress){     // Valid type
		}else if (opper.type == Interpreter::OpperandType::RegisterValue){ // Valid type
		}else if (addressMode){                                                             // Invalid type
			std::cerr << "Error: Invalid opperand B; must be a register value or address since A is an address." << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		//  Check the data is valid
		if (opper.valid == false){
			std::cerr << "Error: Invalid opperand A register." << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << out.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param[3] = static_cast<uint64>(opper.type);
		out.param[4] = opper.data.uint64;


		// Interp operand
		//  Check the type is valid
		opper = Interpreter::Opperand(act.param[4]);
		if (opper.type == Interpreter::OpperandType::RegisterValue){ // Valid type
		}else{                                                       // Invalid type
			std::cerr << "Error: Invalid opperand C; must be a register value or address." << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		//  Check the data is valid
		if (opper.valid == false){
			std::cerr << "Error: Invalid opperand C register." << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param[5] = opper.data.uint64;


		return out;
	}

	// AKA IF
	inline Action InterpGate(RawAction act){
		Action out;
		out.cmd = Command::gate;
		out.line = act.line;

		if (act.param.size() != 2){
			std::cerr << "Error: Invalid number of arguments for if statement" << std::endl;
			std::cerr << "  args: " << ToString(act.param) << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param.resize(2);


		auto opper = Interpreter::Opperand(act.param[1]);
		if (opper.type == Interpreter::OpperandType::RegisterValue){

			if (opper.valid == false){
				std::cerr << "Error: Invalid if statement opperand." << std::endl;
				std::cerr << "  Unknown reason, possibly miss typed value, or invalid type" << std::endl;
				std::cerr << "  arg : " << act.param[1] << std::endl;
				std::cerr << "  line: " << act.line << std::endl;

				out.cmd = Command::invalid;
				return out;
			}

			out.param[0] = static_cast<uint64>(opper.type);
			out.param[1] = static_cast<uint64>(opper.data.uint64);
		}else{
			std::cerr << "Error: Invalid if statement value" << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		};

		return out;
	}
	inline Action InterpGateOther(RawAction act){
		Action out;
		out.cmd = Command::gateOther;
		out.line = act.line;

		if (act.param.size() != 1){
			std::cerr << "Error: Invalid number of arguments for if statement" << std::endl;
			std::cerr << "  args: " << ToString(act.param) << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param.resize(1);

		return out;
	}

	inline Action InterpMath(RawAction act){
		Action out;
		out.cmd = Command::math;
		out.line = act.line;

		if (act.param.size() != 4){
			std::cerr << "Error: Invalid number of arguments for math command" << std::endl;
			std::cerr << "  args: " << ToString(act.param) << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param.resize(5);


		bool addressMode = false;
		auto opper = Interpreter::Opperand(act.param[1]);
		if (opper.type == Interpreter::OpperandType::RegisterAddress || opper.type == Interpreter::OpperandType::RegisterValue){
			addressMode = opper.type == Interpreter::OpperandType::RegisterAddress;

			out.param[0] = static_cast<uint64>(opper.type);
			out.param[1] = static_cast<uint64>(opper.data.uint64);

			if (opper.valid == false){
				std::cerr << "Error: Invalid math opperand A." << std::endl;
				std::cerr << "  Unknown reason, possibly miss typed value, or invalid type" << std::endl;
				std::cerr << "  arg : " << act.param[1] << std::endl;
				std::cerr << "  line: " << act.line << std::endl;

				out.cmd = Command::invalid;
				return out;
			}

		}else{
			std::cerr << "Error: Invalid math opperand A type" << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}

		if       (act.param[2] == "+"){
			out.param[2] = static_cast<uint64>(Interpreter::MathOpperation::addition);
		}else if (act.param[2] == "-"){
			out.param[2] = static_cast<uint64>(Interpreter::MathOpperation::subtract);
		}else if (act.param[2] == "*"){
			out.param[2] = static_cast<uint64>(Interpreter::MathOpperation::multiply);

			if (addressMode){
				std::cerr << "Error: Cannot multiply an address" << std::endl;
				std::cerr << "  args: " << ToString(act.param) << std::endl;
				std::cerr << "  line: " << act.line << std::endl;

				out.cmd = Command::invalid;
				return out;
			}

		}else if (act.param[2] == "/"){
			out.param[2] = static_cast<uint64>(Interpreter::MathOpperation::divide);

			if (addressMode){
				std::cerr << "Error: Cannot divide an address" << std::endl;
				std::cerr << "  args: " << ToString(act.param) << std::endl;
				std::cerr << "  line: " << act.line << std::endl;

				out.cmd = Command::invalid;
				return out;
			}
		}else if (act.param[2] == "%"){
			out.param[2] = static_cast<uint64>(Interpreter::MathOpperation::modulus);

			if (addressMode){
				std::cerr << "Error: Cannot modulo an address" << std::endl;
				std::cerr << "  args: " << ToString(act.param) << std::endl;
				std::cerr << "  line: " << act.line << std::endl;

				out.cmd = Command::invalid;
				return out;
			}
		}

		opper = Interpreter::Opperand(act.param[3]);
		out.param[3] = static_cast<uint64>(opper.type);
		out.param[4] = static_cast<uint64>(opper.data.uint64);

		if (opper.valid == false){
			std::cerr << "Error: Invalid math opperand A." << std::endl;
			std::cerr << "  Unknown reason, possibly miss typed value, or invalid type" << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}

		return out;
	}

	inline Action InterpMode(RawAction act){
		Action out;
		out.cmd = Command::mode;
		out.line = act.line;

		if (act.param.size() != 3){
			std::cerr << "Error: Invalid number of arguments for mode command" << std::endl;
			std::cerr << "  args: " << ToString(act.param) << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param.resize(2);

		// Get target register
		auto reg = GetRegisterID(act.param[1]);
		if (reg == -1){
			std::cerr << "Error: Invalid register supplied to mode command" << std::endl;
			std::cerr << "  arg : " << act.param[1] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}else{
			out.param[0] = reg;
		}

		// Get target type
		if       (act.param[2] == "uint8"){
			out.param[1] = static_cast<uint64>(RegisterMode::uint8);
		}else if (act.param[2] == "int8"){
			out.param[1] = static_cast<uint64>(RegisterMode::int8);
		}else if (act.param[2] == "uint16"){
			out.param[1] = static_cast<uint64>(RegisterMode::uint16);
		}else if (act.param[2] == "int16"){
			out.param[1] = static_cast<uint64>(RegisterMode::int16);
		}else if (act.param[2] == "uint32"){
			out.param[1] = static_cast<uint64>(RegisterMode::uint32);
		}else if (act.param[2] == "int32"){
			out.param[1] = static_cast<uint64>(RegisterMode::int32);
		}else if (act.param[2] == "uint64"){
			out.param[1] = static_cast<uint64>(RegisterMode::uint64);
		}else if (act.param[2] == "int64"){
			out.param[1] = static_cast<uint64>(RegisterMode::int64);
		}else if (act.param[2] == "float32"){
			out.param[1] = static_cast<uint64>(RegisterMode::float32);
		}else if (act.param[2] == "float64"){
			out.param[1] = static_cast<uint64>(RegisterMode::float64);
		}else{
			std::cerr << "Error: Invalid type supplied to mode command" << std::endl;
			std::cerr << "  arg : " << act.param[2] << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}


		return out;
	}

	inline Action InterpRtrn(RawAction act){
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

	inline Action InterpSet(RawAction act){
		Action out;
		out.cmd = Command::set;
		out.line = act.line;

		if (act.param.size() != 3){
			std::cerr << "Error: Invalid number of arguments for set command" << std::endl;
			std::cerr << "  args: " << ToString(act.param) << std::endl;
			std::cerr << "  line: " << act.line << std::endl;

			out.cmd = Command::invalid;
			return out;
		}
		out.param.resize(5);


		// Interpret target
		auto opper = Interpreter::Opperand(act.param[1]);
		bool addressMode = opper.type == Interpreter::OpperandType::RegisterAddress;
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
		if       (act.param[2] == "local"){
			out.param[2] = 1;
			out.param[3] = 1;

			if (opper.type != Interpreter::OpperandType::RegisterAddress){
				std::cerr << "Error: Setting to 'local' requires the target to be a register address" << std::endl;
				std::cerr << "  line: " << act.line << std::endl;
				out.cmd = Command::invalid;
			return out;
			}
		}else if (act.param[2] == "parse"){
			out.param[2] = 1;
			out.param[3] = 2;

			if (opper.type != Interpreter::OpperandType::RegisterAddress){
				std::cerr << "Error: Setting to 'parse' requires the target to be a register address" << std::endl;
				std::cerr << "  line: " << act.line << std::endl;
				out.cmd = Command::invalid;
				return out;
			}
		}else{
			opper = Interpreter::Opperand(act.param[2]);

			// Changing the value or address of a register?
			if (addressMode){
				// Cannot set the address of a register to a constant value
				if (opper.type != Interpreter::OpperandType::RegisterAddress && opper.type != Interpreter::OpperandType::RegisterValue){
					std::cerr << "Error: Invalid value type of set command" << std::endl;
					std::cerr << "   arg: " << act.param[2] <<std::endl;
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
			}
			out.param[2] = 0;
			out.param[3] = static_cast<uint64>(opper.type);
			out.param[4] = opper.data.uint64;
		}


		return out;
	};


	// Forwards interpretation to dedicated functions
	Action Convert(RawAction act, Function* context){
		Action out;
		out.cmd = Command::invalid;

		switch ( CommandFrom(act.param[0]) ){
			case Command::invalid:
				std::cerr << "Error: Unknown commmand" << std::endl;
				std::cerr << "  cmd : " << act.param[0] << std::endl;
				std::cerr << "  line: " << act.line << std::endl;

				break;
			case Command::set:
				out = InterpSet(act);
				break;
			case Command::rtrn:
				out = InterpRtrn(act);
				break;
			case Command::math:
				out = InterpMath(act);
				break;
			case Command::gate:
				out = InterpGate(act);
				break;
			case Command::gateOther:
				out = InterpGateOther(act);
				break;
			case Command::mode:
				out = InterpMode(act);
				break;
			case Command::compare:
				out = InterpCompare(act);
				break;
			case Command::blockOpen:
				// Overflow into blockClose
			case Command::blockClose:
				out = InterpBlock(act);
				break;
		}

		return out;
	};
}
