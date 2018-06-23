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
bool Function::Parse(Segregate::StrCommands source){  
  int codeLen = source.size();
  this->code.resize( codeLen );

  bool error = false;          // Has an error been detected?
  int ptr = 0;                 // Which index to place the next action

  for (int i=0; i<codeLen; i++){

    // Ignore empty lines
    if (source[i].size() == 0){
      continue;
    }

    // Ignore comment lines
    if (source[i][0] == "#"){
      continue;
    }

    // Stop
    if (source[i][0] == "stop"){
      this->code[ptr].command = Commands::stop;
      this->code[ptr].line = i;
      this->code[ptr].param.resize(0);

      ptr++;
      continue;
    };

    // Set Register
    if (source[i][0] == "set"){
      if (source[i].size() < 3){
        std::cerr << "Error: Not enough arguments supplied (line: "<< i <<")"<<std::endl;
        error = true;
      }

      this->code[ptr].command = Commands::set;                               // Command ID
      this->code[ptr].line = i;
      this->code[ptr].param.resize(3);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);                // Register to be written

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Supplied bad target register '"<<source[i][1]<<"' (line: "<<i<<")" << std::endl;
        error = true;
      }
      
      if (source[i][2][0] == 'r'){
        this->code[ptr].param[1] = 1;
        this->code[ptr].param[2] = GetRegisterID(source[i][2]);              // Constant value

        if (this->code[ptr].param[2] == -1){
          std::cerr << "Error: Supplied bad value register '"<<source[i][2]<<"' (line: "<<i<<")" << std::endl;
          error = true;
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
        std::cerr << "Error: Not enough arguments supplied (line: "<< i <<")";
        error = true;
      }

      this->code[ptr].command = Commands::push;
      this->code[ptr].line = i;
      this->code[ptr].param.resize(2);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      this->code[ptr].param[1] = GetRegisterID(source[i][2]);

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Supplied bad value register '"<<source[i][1]<<"' (line: "<<i<<")" << std::endl;
        error = true;
      }
      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Supplied bad address register '"<<source[i][2]<<"' (line: "<<i<<")" << std::endl;
        error = true;
      }

      ptr++;
      continue;
    }

    // Push register data to loc
    if (source[i][0] == "pull"){
      if (source[i].size() < 3){
        std::cerr << "Error: Not enough arguments supplied (line: "<< i <<")";
        error = true;
      }

      this->code[ptr].command = Commands::pull;
      this->code[ptr].line = i;
      this->code[ptr].param.resize(2);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      this->code[ptr].param[1] = GetRegisterID(source[i][2]);

      if (this->code[ptr].param[0] == -1){
        std::cerr << "Error: Supplied bad value register '"<<source[i][1]<<"' (line: "<<i<<")" << std::endl;
        error = true;
      }
      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Supplied bad address register '"<<source[i][2]<<"' (line: "<<i<<")" << std::endl;
        error = true;
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
      }

      this->code[ptr].command = Commands::memory;
      this->code[ptr].line = i;
      this->code[ptr].param.resize(2);

      if (source[i][1] == "alloc"){
        this->code[ptr].param[0] = 0;
      }else if (source[i][1] == "unalloc"){
        this->code[ptr].param[0] = 1;
      }else{
        error = true;
        std::cerr << "Error: Unknown memory action '" << source[i][1] << "'" << std::endl;
        std::cerr << "  line: " << i << std::endl; 
      }

      this->code[ptr].param[1] = GetRegisterID(source[i][2]);
      this->code[ptr].param[2] = GetRegisterID(source[i][3]);

      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Supplied bad address register '"<<source[i][2]<<"' (line: "<<i<<")" << std::endl;
        error = true;
      }
      if (this->code[ptr].param[1] == -1){
        std::cerr << "Error: Supplied bad length register '"<<source[i][3]<<"' (line: "<<i<<")" << std::endl;
        error = true;
      }

      ptr++;
      continue;
    }

    // Standard Stream
    if (source[i][0] == "ss"){
      if (source[i].size() < 4){
        std::cerr << "Error: Not enough arguments supplied (line: "<< i <<")";
        error = true;
      }

      this->code[ptr].command = Commands::standardStream;
      this->code[ptr].line = i;
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
        std::cerr << "Error: Supplied bad address register '"<<source[i][2]<<"' (line: "<<i<<")" << std::endl;
        error = true;
      }
      if (this->code[ptr].param[2] == -1){
        std::cerr << "Error: Supplied bad length register '"<<source[i][3]<<"' (line: "<<i<<")" << std::endl;
        error = true;
      }

      ptr++;
      continue;
    }

    // Switch Register Mode
    if (source[i][0] == "mode"){
      if (source[i].size() < 3){
        std::cerr << "Error: Not enough arguments supplied (line: "<< i <<")";
        error = true;
      }

      this->code[ptr].command = Commands::mode;
      this->code[ptr].line = i;
      this->code[ptr].param.resize(2);

      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
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
      }

      ptr++;
      continue;
    }

    // Switch Register Mode, translating the data accordingly
    if (source[i][0] == "translate"){
      if (source[i].size() < 3){
        std::cerr << "Error: Not enough arguments supplied (line: "<< i <<")";
        error = true;
      }

      this->code[ptr].command = Commands::translate;
      this->code[ptr].line = i;
      this->code[ptr].param.resize(2);

      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
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
      }

      ptr++;
      continue;
    }

    if (source[i][0] == "math"){
      this->code[ptr].command = Commands::math;
      this->code[ptr].line = i;
      this->code[ptr].param.resize(4);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      this->code[ptr].param[2] = GetRegisterID(source[i][3]);
      this->code[ptr].param[3] = GetRegisterID(source[i][4]);

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
        ptr -= 1;
      }
      
      ptr++;
      continue;
    }

    if (source[i][0] == "copy"){
      this->code[ptr].command = Commands::copy;
      this->code[ptr].line = i;
      this->code[ptr].param.resize(2);
      this->code[ptr].param[0] = GetRegisterID(source[i][1]);
      this->code[ptr].param[1] = GetRegisterID(source[i][2]);

      ptr++;
      continue;
    }



    std::cerr << "Error: Invalid Command '" << source[i][0] << "'"<<std::endl;
    std::cerr << "  Line: "<<i<<std::endl;
    error = true;
  }




  // Shrink the vector to the correct size
  this->code.resize(ptr);

  if (error){
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