#include "function.h"





int Function::GetChildsID(std::string str){
  int length = this->children.size();
  for (int i=0; i<length; i++){
    if (this->children[i]->name == str){
      return i;
    }
  }
  
  return -1;
};
Function::Function(std::string name, int size){
  this->name = name;
  this->size = size;
}
bool Function::Interpret(Segregate::StrCommands source){
  unsigned int codeLen = source.size();
  this->code.resize( codeLen );

  bool error = false;          // Has an error been detected?
  int ptr = 0;                 // Which index to place the next action

  for (unsigned int i=0; i<codeLen; i++){

    // Ignore empty lines
    if (source[i].size() == 0){
      continue;
    }

    // Ignore comment lines
    if (source[i][0] == "#"){
      continue;
    }
    if (source[i][0] == ";"){
      continue;
    }

    // Stop
    if (source[i][0] == "stop"){
      this->code[ptr].command = Commands::stop;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(0);

      ptr++;
      continue;
    };

    // Set Register
    if (source[i][0] == "set"){
      if (source[i].size() < 3){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::set;                               // Command ID
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(3);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);                // Register to be written

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Supplied bad target register "<<source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      
      if (source[i][2][0] == 'r'){
        this->code[ptr].param[1] = 1;
        this->code[ptr].param[2] = GetRegisterID(source[i][2]);              // Constant value

        if (this->code[ptr].param[2] == -1){
          std::cerr << "Error: Supplied bad value register "<<source[i][2] << std::endl;
          std::cerr << "  line: " << i+1 << std::endl;
          error = true;
          continue;
        }
      }else{
        this->code[ptr].param[1] = 0;
        this->code[ptr].param[2] = Hexidecimal::convert(source[i][2]);       // Constant value
      }

      ptr++;
      continue;
    }

    // Pull data from loc into register
    if (source[i][0] == "push"){
      if (source[i].size() < 3){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::push;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(2);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      this->code[ptr].param[1] = GetRegisterID(source[i][2]);

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Supplied bad value register '"<<source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Supplied bad address register '"<<source[i][2] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      ptr++;
      continue;
    }

    // Push register data to loc
    if (source[i][0] == "pull"){
      if (source[i].size() < 3){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::pull;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(2);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      this->code[ptr].param[1] = GetRegisterID(source[i][2]);

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Supplied bad value register '"<<source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Supplied bad address register '"<<source[i][2] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      ptr++;
      continue;
    }
    
    // (Un)Allocate command
    if (source[i][0] == "mem"){
      if (source[i].size() < 4){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: " << i << std::endl; 
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::memory;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(3);

      if (source[i][1] == "alloc"){
        this->code[ptr].param[0] = 0;
      }else if (source[i][1] == "unalloc"){
        this->code[ptr].param[0] = 1;
      }else{
        error = true;
        std::cerr << "Error: Unknown memory action '" << source[i][1] << "'" << std::endl;
        std::cerr << "  line: " << i << std::endl; 
        continue;
      }

      this->code[ptr].param[1] = GetRegisterID(source[i][2]);
      this->code[ptr].param[2] = GetRegisterID(source[i][3]);

      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Supplied bad address register '"<<source[i][2] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Supplied bad length register '"<<source[i][3] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      ptr++;
      continue;
    }

    // Standard Stream
    if (source[i][0] == "ss"){
      if (source[i].size() < 4){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::standardStream;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(3);

      // Mode
      if (source[i][1] == "in"){
        this->code[ptr].param[0] = 0;
      }else if (source[i][1] == "out"){
        this->code[ptr].param[0] = 1;
      }else if (source[i][1] == "err"){
        this->code[ptr].param[0] = 2;
      }else if (source[i][1] == "log"){
        this->code[ptr].param[0] = 3;
      }

      this->code[ptr].param[1] = GetRegisterID(source[i][2]);
      this->code[ptr].param[2] = GetRegisterID(source[i][3]);
      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Supplied bad address register '"<<source[i][2] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[2] == -1){
        std::cerr << "Error: Supplied bad length register '"<<source[i][3] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      ptr++;
      continue;
    }

    // Switch Register Mode
    if (source[i][0] == "mode"){
      if (source[i].size() < 3){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::mode;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(2);

      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Invalid target register "<<source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      if       (source[i][2] == "uint8"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint8);
      }else if (source[i][2] == "int8"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int8);
      }else if (source[i][2] == "uint16"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint16);
      }else if (source[i][2] == "int16"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int16);
      }else if (source[i][2] == "float32"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::float32);
      }else if (source[i][2] == "uint32"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint32);
      }else if (source[i][2] == "int32"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int32);
      }else if (source[i][2] == "float64"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::float64);
      }else if (source[i][2] == "uint64"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint64);
      }else if (source[i][2] == "int64"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int64);
      }

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Supplied bad target register '"<<source[i][1]<<"' (line: "<<i<<")" << std::endl;
        error = true;
        continue;
      }

      ptr++;
      continue;
    }

    // Switch Register Mode, translating the data accordingly
    if (source[i][0] == "translate"){
      if (source[i].size() < 3){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::translate;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(2);

      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Invalid target register "<<source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      if       (source[i][2] == "uint8"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint8);
      }else if (source[i][2] == "int8"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int8);
      }else if (source[i][2] == "uint16"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint16);
      }else if (source[i][2] == "int16"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int16);
      }else if (source[i][2] == "float32"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::float32);
      }else if (source[i][2] == "uint32"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint32);
      }else if (source[i][2] == "int32"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int32);
      }else if (source[i][2] == "float64"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::float64);
      }else if (source[i][2] == "uint64"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::uint64);
      }else if (source[i][2] == "int64"){
        this->code[ptr].param[1] = static_cast<unsigned long long int>(RegisterMode::int64);
      }

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Supplied bad target register '"<<source[i][1]<<"' (line: "<<i<<")" << std::endl;
        error = true;
        continue;
      }

      ptr++;
      continue;
    }

    // Math operations
    if (source[i][0] == "math"){
      if (source[i].size() < 5){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::math;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(4);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      this->code[ptr].param[2] = GetRegisterID(source[i][3]);
      this->code[ptr].param[3] = GetRegisterID(source[i][4]);

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Invalid A register " << source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[2] == -1){
        std::cerr << "Error: Invalid B register " << source[i][2] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[3] == -1){
        std::cerr << "Error: Invalid result register" << source[i][3] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      if (source[i][2] == "add"){
        this->code[ptr].param[1] = MathOpperation::add;
      }else if (source[i][2] == "subtract"){
        this->code[ptr].param[1] = MathOpperation::subtract;
      }else if (source[i][2] == "multiply"){
        this->code[ptr].param[1] = MathOpperation::multiply;
      }else if (source[i][2] == "divide"){
        this->code[ptr].param[1] = MathOpperation::divide;
      }else if (source[i][2] == "modulus"){
        this->code[ptr].param[1] = MathOpperation::modulus;
      }else if (source[i][2] == "exponent"){
        this->code[ptr].param[1] = MathOpperation::exponent;
      }else{
        std::cerr << "Error: Invalid math operation \""<< source[i][2] << "\"" <<std::endl;
        std::cerr << "  Line: "<<i<<std::endl;
        error = true;
        continue;
      }
      
      ptr++;
      continue;
    }

    // Duplicate the properties of one register to another
    if (source[i][0] == "copy"){
      if (source[i].size() < 3){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::copy;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(2);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      this->code[ptr].param[1] = GetRegisterID(source[i][2]);

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Invalid from register " << source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Invalid to register" << source[i][2] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      ptr++;
      continue;
    }

    // Duplicate data from one position to another
    if (source[i][0] == "move"){
      if (source[i].size() < 4){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::move;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(3);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      this->code[ptr].param[1] = GetRegisterID(source[i][2]);
      this->code[ptr].param[2] = GetRegisterID(source[i][3]);

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Invalid from register " << source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Invalid to register " << source[i][2] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[2] == -1){
        std::cerr << "Error: Invalid length register " << source[i][3] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }


      ptr++;
      continue;
    }

    // Compare two register's numetric value
    if (source[i][0] == "compare"){
      if (source[i].size() < 5){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::compare;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(4);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Invalid A register " << source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      if       (source[i][2] == "="){
        this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::equal);
      }else if (source[i][2] == ">"){
        this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::greater);
      }else if (source[i][2] == "<"){
        this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::less);
      }else{
        std::cerr << "Error: Invalid comparason operation \""<< source[i][2] << "\"" <<std::endl;
        std::cerr << "  Line: "<<i<<std::endl;
        error = true;
        continue;
      }

      this->code[ptr].param[2] = GetRegisterID(source[i][3]);
      this->code[ptr].param[3] = GetRegisterID(source[i][4]);

      if (this->code[ptr].param[2] == -1){
        std::cerr << "Error: Invalid B register " << source[i][3] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[3] == -1){
        std::cerr << "Error: Invalid result register " << source[i][4] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      ptr++;
      continue;
    }

    // Compare two chunks of bytes
    if (source[i][0] == "lcompare"){
      if (source[i].size() < 5){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< this->code[ptr].line << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::compare;
      this->code[ptr].line = (i+1);
      this->code[ptr].param.resize(6);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Invalid A register " << source[i][1] << std::endl;
        std::cerr << "  line: " << this->code[ptr].line << std::endl;
        error = true;
        continue;
      }

      if       (source[i][2] == "="){
        this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::equal);
      }else if (source[i][2] == ">"){
        this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::greater);
      }else if (source[i][2] == "<"){
        this->code[ptr].param[1] = static_cast<unsigned long long>(Comparason::less);
      }else{
        std::cerr << "Error: Invalid comparason operation \""<< source[i][2] << "\"" <<std::endl;
        std::cerr << "  Line: "<< this->code[ptr].line << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].param[2] = GetRegisterID(source[i][3]);
      this->code[ptr].param[3] = GetRegisterID(source[i][4]);

      if (this->code[ptr].param[2] == -1){
        std::cerr << "Error: Invalid B register " << source[i][3] << std::endl;
        std::cerr << "  line: " << this->code[ptr].line << std::endl;
        error = true;
        continue;
      }
      if (this->code[ptr].param[3] == -1){
        std::cerr << "Error: Invalid result register " << source[i][4] << std::endl;
        std::cerr << "  line: " << this->code[ptr].line << std::endl;
        error = true;
        continue;
      }


      if (source[i][5] == "little"){
        this->code[ptr].param[4] = 0;
      }else if (source[i][5] == "big"){
        this->code[ptr].param[4] = 1;
      }else{
        std::cerr << "Error: Invalid edianess; " << source[i][5] << std::endl;
        std::cerr << "  line: " << this->code[ptr].line << std::endl;
        error = true;
        continue;
      }


      this->code[ptr].param[5] = GetRegisterID(source[i][4]);
      if (this->code[ptr].param[5] == -1){
        std::cerr << "Error: Invalid length register " << source[i][5] << std::endl;
        std::cerr << "  line: " << this->code[ptr].line << std::endl;
        error = true;
        continue;
      }

      ptr++;
      continue;
    }

    // Bitwise operations
    if (source[i][0] == "bit"){
      if (source[i].size() < 5){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::bitwise;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(5);

      // A
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Invalid A register " << source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      // Operation
      if       (source[i][1] == "&"){
        this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::AND);
      }else if (source[i][1] == "|"){
        this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::OR);
      }else if (source[i][1] == "^"){
        this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::XOR);
      }else if (source[i][1] == "<<"){
        this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::LeftShift);
      }else if (source[i][1] == ">>"){
        this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::RightShift);
      }else if (source[i][1] == "~"){
        this->code[ptr].param[1] = static_cast<unsigned long long>(BitOperator::NOT);
      }
      // B
      if (source[i][2][0] == 'r'){
        this->code[ptr].param[2] = 1;
        this->code[ptr].param[3] = GetRegisterID(source[i][3]);

        if (this->code[ptr].param[0] == -1){
          std::cerr << "Error: Invalid B register " << source[i][3] << std::endl;
          std::cerr << "  line: " << i+1 << std::endl;
          error = true;
          continue;
        }
      }else{
        this->code[ptr].param[2] = 0;
        this->code[ptr].param[3] = Hexidecimal::convert(source[i][3]);       // Constant value
      }

      // Result Store
      this->code[ptr].param[4] = GetRegisterID(source[i][4]);
      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Invalid result register " << source[i][4] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
        continue;
      }

      ptr++;
      continue;
    }


    // IF statement
    if (source[i][0] == "if"){
      if (source[i].size() < 2){
        std::cerr << "Error: Not enough arguments supplied" << std::endl;
        std::cerr << "  line: "<< i+1 << std::endl;
        error = true;
        continue;
      }

      this->code[ptr].command = Commands::IF;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(2);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      // param[1] = jump else statement

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Invalid boolean register " << source[i][1] << std::endl;
        std::cerr << "  line: " << i+1 << std::endl;
        error = true;
      }

      ptr++;
      continue;
    }
    if (source[i][0] == "else"){
      this->code[ptr].command = Commands::ELSE;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(2);

      ptr++;
      continue;
    }
    if (source[i][0] == "end"){
      this->code[ptr].command = Commands::END;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(0);

      ptr++;
      continue;
    }

    // Flow Navigation
    if (source[i][0] == "break"){
      this->code[ptr].command = Commands::Break;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(0);
      // param[0] = distance to loop/switch end (command num)

      ptr++;
      continue;
    }
    if (source[i][0] == "continue"){
      this->code[ptr].command = Commands::Continue;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(0);
      // param[0] = distance to loop beginning (command num)

      ptr++;
      continue;
    }


    // Loop statement
    if (source[i][0] == "loop"){
      this->code[ptr].command = Commands::Loop;
      this->code[ptr].line = i+1;
      this->code[ptr].param.resize(1);
      // param[0] = distance to loop end (command num)

      ptr++;
      continue;
    }
    

    std::cerr << "Error: Invalid Command '" << source[i][0] << "'"<<std::endl;
    std::cerr << "  Line: "<<i+1<<std::endl;
    error = true;
  }




  // Shrink the vector to the correct size
  this->code.resize(ptr);

  if (error){
    return false;
  }

  return true;
};

bool Function::SimplifyIF(){
  unsigned int elseLoc;  // Where the Else clause starts
  bool hasElse;          // Does the statement have an else clause

  unsigned int depth = 0; // Ignore inner IF content while scanning for else/endif

  unsigned int length = this->code.size();
  unsigned int i;
  unsigned int j;
  for (i=0; i<length; i++){

    if (this->code[i].command == Commands::IF){
      depth = 0;
      hasElse = false;

      // Find the extra statements
      for (j=0; j<length; j++){
        if (
          this->code[j].command == Commands::IF ||
          this->code[j].command == Commands::Loop ||
          this->code[j].command == Commands::Switch
        ){
          depth += 1;
        }

        if (this->code[j].command == Commands::ELSE){
          elseLoc = j;
          hasElse = true;
        }

        if (this->code[j].command == Commands::END){
          depth -= 1;

          if (depth == 0){
            break;
          }
        }
      }

      // How far to skip on the false statement
      if (hasElse){
        // Has an else statement,

        // So ensure that the true statement will not overflow in to the else
        this->code[elseLoc].command = Commands::jump;
        this->code[elseLoc].param[0] = 1;
        this->code[elseLoc].param[1] = j-elseLoc;

        // Give the if statement a jump to point for the false condition
        this->code[i].param[1] = elseLoc-i; // +1: Don't run the else overflow
      }else{
        // No else statement, so just skip to the end
        this->code[i].param[1] = j-i;
      }
      this->code[j].command = Commands::blank;
    }

  }

  return true;
};

bool Function::SimplifyLoop(){
  unsigned long length = this->code.size();
  unsigned long depth = 0;
  bool found = false;

  // Specify loops with their lengths
  // Make loop endpoints wrap
  for (unsigned long i=0; i<length; i++){
    if (this->code[i].command == Commands::Loop){
      depth = 1;
      found = false;

      for (unsigned long j=0; j<length; j++){

        // Ensure depth
        if (
          this->code[j].command == Commands::IF ||
          this->code[j].command == Commands::Loop ||
          this->code[j].command == Commands::Switch
        ){
          depth += 1;
          continue;
        }
        

        if (this->code[j].command == Commands::END){
          depth -= 1;
        }

        if (depth == 0){
          this->code[i].param[0] = j-i;

          // Change the end mark of the while loop
          // To a backwards jump to continue the loop
          this->code[j].command = Commands::jump;
          this->code[j].param.resize(2);
          this->code[j].param[0] = 0;
          this->code[j].param[1] = this->code[i].param[0];
          found = true;

          break;
        }
      }

      if (!found){
        std::cerr << "Error: Missing loop endpoint"<<std::endl;
        std::cerr << "  Line: "<<this->code[i].line<<std::endl;
        return false;
      }
    }
  }

  // Bind continue command
  for (unsigned long i=0; i<length; i++){
    if (this->code[i].command == Commands::Continue){
      depth = this->code[i].param[0];
      found = false;

      for (long j=i-1; j>=0; j--){

        // Change of depth
        //   Note: Switch statements don't have 'continue'
        if(
          this->code[j].command == Commands::Loop  &&
          this->code[j].param[0] >= i-j               // Loop covers keyword
        ){
          depth -= 1;
        }

        if (depth == 0){
          this->code[i].command = Commands::jump;
          this->code[i].param.resize(2);
          this->code[i].param[0] = 0;
          this->code[i].param[1] = i-j;
          found = true;

          break;
        }
      }

      if (!found){
        std::cerr << "Error: Failed to find loop with correct depth for continue statement (" << this->code[i].param[0] << ")"<<std::endl;
        std::cerr << "  Line: "<<this->code[i].line<<std::endl;
        return false;
      }
    }
  }

  // Bind break command
  for (unsigned long i=0; i<length; i++){
    if (this->code[i].command == Commands::Break){
      depth = this->code[i].param[0];
      found = false;

      for (long j=i-1; j>=0; j--){

        // Change of depth
        if (this->code[j].command == Commands::Switch){
          depth -= 1;
        }else if(
          this->code[j].command == Commands::Loop  &&
          this->code[j].param[0] >= i-j               // Loop covers keyword
        ){
          depth -= 1;
        }

        if (depth == 0){
          this->code[i].command = Commands::jump;
          this->code[i].param.resize(2);
          this->code[i].param[0] = 0;
          this->code[i].param[1] = this->code[j].param[0] - (j - i);
          found = true;

          break;
        }
      }

      if (!found){
        std::cerr << "Error: Failed to find loop with correct depth for break statement (" << this->code[i].param[0] << ")"<<std::endl;
        std::cerr << "  Line: "<<this->code[i].line<<std::endl;
        return false;
      }
    }
  }


  return true;
};


bool Function::Parse(Segregate::StrCommands src){
  if (this->Interpret(src) == false){
    return false;
  }
  if (this->SimplifyIF() == false){
    return false;
  }

  return true;
};





bool ValidTypeSizing(){
  Handle test;

  if (sizeof(test.int8) != 1){
    std::cerr <<  "Invalid int8 size" << std::endl;
    std::cerr << "  "<<sizeof(test.int8)<<" != 8" << std::endl;
    return false;
  }
  if (sizeof(test.int16) != 2){
    std::cerr <<  "Invalid int16 size" << std::endl;
    std::cerr << "  "<<sizeof(test.int16)<<" != 8" << std::endl;
    return false;
  }
  if (sizeof(test.int32) != 4){
    std::cerr <<  "Invalid int32 size" << std::endl;
    std::cerr << "  "<<sizeof(test.int32)<<" != 8" << std::endl;
    return false;
  }
  if (sizeof(test.float32) != 4){
    std::cerr <<  "Invalid float32 size" << std::endl;
    std::cerr << "  "<<sizeof(test.float32)<<" != 8" << std::endl;
    return false;
  }
  if (sizeof(test.int64) != 8){
    std::cerr <<  "Invalid int64 size" << std::endl;
    std::cerr << "  "<<sizeof(test.int64)<<" != 8" << std::endl;
    return false;
  }
  if (sizeof(test.float64) != 8){
    std::cerr <<  "Invalid float64 size" << std::endl;
    std::cerr << "  "<<sizeof(test.float64)<<" != 8" << std::endl;
    return false;
  }

  if (sizeof(test.address) > 8){
    std::cerr << "Invalid address size" << std::endl;
    std::cerr << "  "<<sizeof(test.address)<<" > 8" << std::endl;
    return false;
  }

  return true;
}