#include "instance.h"




Instance::Instance (Function *func, Instance *caller){
  this->ref = func;
  this->parent = caller;
};
void Instance::Execute (int cursor){
  int length = this->ref->code.size();
  Action *act;

  while (cursor < length){
    act = &this->ref->code[cursor];

    // std::cout << act->line << " > Command[" << act->command << ']' << std::endl;

    switch (act->command){
      case Commands::math:
        this->CmdMath(act);
        break;
      case Commands::set:
        this->CmdSet(act);
        break;
      case Commands::push:
        this->CmdPush(act);
        break;
      case Commands::pull:
        this->CmdPull(act);
        break;
      case Commands::mode:
        this->CmdMode(act);
        break;
      case Commands::copy:
        this->CmdCopy(act);
        break;
      case Commands::translate:
        this->CmdTranslate(act);
        break;
      case Commands::standardStream:
        this->CmdSS(act);
        break;
      case Commands::memory:
        this->CmdMem(act);
        break;
      case Commands::stop:
          return;
        break;
      case Commands::invalid:
      default:
        std::cerr << "Warn: Unexpected invalid command" << std::endl;
        break;
    }

    cursor += 1;
  }
};




/*------------------------------------------
    Commands
------------------------------------------*/
void Instance::CmdSet       (Action *act){
  if (act->param[1] == 1){ // Copy register values across
    this->handle[ act->param[0] ].write(  this->handle[ act->param[2] ].read()  );
  }else{                   // Set the literal value of a register
    this->handle[ act->param[0] ].write( act->param[2] );
  }
};
void Instance::CmdSS        (Action *act){
  std::vector<char> bytes;
  unsigned long count;
  std::string str;
  Address ptr;

  if (this->handle[ act->param[1] ].mode != RegisterMode::uint64){
    std::cerr << "Warn: Attempting to use standard stream with an address register of non-uint64 mode" << std::endl;
    std::cerr << "  Mode: "<<this->handle[ act->param[1] ].mode                                        << std::endl;
    std::cerr << "  Line: "<<act->line                                                                 << std::endl;
  }

  if (act->param[0] == 0){
    if (this->handle[ act->param[2] ].mode != RegisterMode::uint64){
      std::clog << "Warn: Standard Stream length result register should be in uint64 mode to prevent overloading" << std::endl;
      std::clog << "  Mode: " << this->handle[ act->param[2] ].mode                                               << std::endl;
      std::clog << "  Line: " << act->line                                                                        <<std::endl;
    }
    
    std::cin >> str;
    count = sizeof(str);

    // Create a safe space for the input stream data
    //   (Won't get destroyed after function call)
    // Store the information about the space in the 
    // two specified registers
    this->handle[ act->param[1] ].value.address = (Handle *) malloc ( count );
    this->handle[ act->param[2] ].write(count);

    // Move the data to the space space
    // memcpy(this->handle[ act->param[1] ].value.address, &str, count );
    memmove(this->handle[ act->param[1] ].value.address, &str, count );

  }else{
    count = this->handle[ act->param[2] ].read();
    ptr.h = this->handle[ act->param[1] ].value.address;

    bytes.resize(count);

    for (unsigned long i=0; i<count; i++){
      bytes[i] = *(ptr.c + i);
    }

    std::string str(bytes.begin(), bytes.end());
    switch (act->param[0]){
      case 1:
        std::cout << str;
        break;
      case 2:
        std::cerr << str;
        break;
      case 3:
        std::clog << str;
        break;
    }
  }
};
void Instance::CmdMem       (Action *act){
  if (this->handle[ act->param[1] ].mode != RegisterMode::uint64){
    std::cerr << "Warn: Attempting to (un)allocate memory with an address register of non-uint64 mode" << std::endl;
    std::cerr << "  Mode: "<<this->handle[ act->param[1] ].mode<<std::endl;
    std::cerr << "  Line: "<<act->line<<std::endl;
  }

  if (act->param[0] == 0){
    this->handle[ act->param[1] ].value.address = (Handle *) malloc ( this->handle[ act->param[2] ].read() );
  }else{
    free ( this->handle[ act->param[1] ].value.address );
  }
};
void Instance::CmdPush      (Action *act){
  Handle *director;
  director = this->handle[ act->param[1] ].value.address;

  switch (this->handle[ act->param[0] ].mode){
    case RegisterMode::int8:
    case RegisterMode::uint8:
      director->uint8 = this->handle[ act->param[0] ].value.uint8;
      break;

    case RegisterMode::int16:
    case RegisterMode::uint16:
      director->uint16 = this->handle[ act->param[0] ].value.uint16;
      break;

    case RegisterMode::int32:
    case RegisterMode::uint32:
    case RegisterMode::float32:
      director->uint32 = this->handle[ act->param[0] ].value.uint32;
      break;

    case RegisterMode::int64:
    case RegisterMode::uint64:
    case RegisterMode::float64:
      director->uint64 = this->handle[ act->param[0] ].value.uint64;
      break;
  }
};
void Instance::CmdPull      (Action *act){
  Handle *director;
  director = this->handle[ act->param[1] ].value.address;

  switch (this->handle[ act->param[0] ].mode){
    case RegisterMode::int8:
    case RegisterMode::uint8:
      this->handle[ act->param[0] ].value.uint8 = director->uint8;

    case RegisterMode::int16:
    case RegisterMode::uint16:
      this->handle[ act->param[0] ].value.uint16 = director->uint16;

    case RegisterMode::int32:
    case RegisterMode::uint32:
    case RegisterMode::float32:
      this->handle[ act->param[0] ].value.uint32 = director->uint32;

    case RegisterMode::int64:
    case RegisterMode::uint64:
    case RegisterMode::float64:
      this->handle[ act->param[0] ].value.uint64 = director->uint64;
  }
};
void Instance::CmdMode      (Action *act){  
  this->handle[ act->param[0] ].mode = static_cast<RegisterMode>(act->param[1]);
};
void Instance::CmdTranslate (Action *act){
  this->handle[ act->param[0] ].Translate( static_cast<RegisterMode>(act->param[1]) );
};
void Instance::CmdMath      (Action *act){
  Register A;
  Register B;
  Register *C;
  RegisterMode goal;

  A.mode = this->handle[ act->param[0] ].mode;
  A.write( this->handle[ act->param[0] ].read() );
  B.mode = this->handle[ act->param[2] ].mode;
  B.write( this->handle[ act->param[2] ].read() );
  C = &this->handle[ act->param[3] ];
  goal = C->mode; // Ensure the output type is never lost

  switch ( static_cast<MathOpperation>(act->param[1]) ){
    case MathOpperation::add:
      A.Translate( C->mode );
      B.Translate( C->mode );
      
      if (C->mode == RegisterMode::float32){
        C->value.float32 = A.value.float32 + B.value.float32;
      }else if (C->mode == RegisterMode::float64){
        C->value.float64 = A.value.float64 + B.value.float64;
      }else{
        auto temp1 = A.read();
        auto temp2 = B.read();
        auto temp3 = temp1 + temp2;
        C->write(temp3);
      }
      break;
    case MathOpperation::subtract:
      A.Translate( C->mode );
      B.Translate( C->mode );
      
      if (C->mode == RegisterMode::float32){
        C->value.float32 = A.value.float32 + B.value.float32;
      }else if (C->mode == RegisterMode::float64){
        C->value.float64 = A.value.float64 + B.value.float64;
      }else{
        C->write( A.read() - B.read() );
      }
      break;
    case MathOpperation::multiply:
      if (A.mode == RegisterMode::float32 || A.mode == RegisterMode::float64 || B.mode == RegisterMode::float32 || B.mode == RegisterMode::float64){
        A.Translate(RegisterMode::float64);
        B.Translate(RegisterMode::float64);
        C->mode = RegisterMode::float64;

        C->value.float64 = A.value.float64 * B.value.float64;
        C->Translate(goal);
      }else{
        C->write( A.read() * B.read() );
      }
      break;
    case MathOpperation::divide:
      if (A.mode == RegisterMode::float32 || A.mode == RegisterMode::float64 || B.mode == RegisterMode::float32 || B.mode == RegisterMode::float64){
        A.Translate(RegisterMode::float64);
        B.Translate(RegisterMode::float64);
        C->mode = RegisterMode::float64;

        C->value.float64 = A.value.float64 / B.value.float64;
        C->Translate(goal);
      }else{
        C->write( A.read() / B.read() );
      }
      break;
    case MathOpperation::modulus:
      if (A.mode == RegisterMode::float32 || A.mode == RegisterMode::float64 || B.mode == RegisterMode::float32 || B.mode == RegisterMode::float64){
        A.Translate(RegisterMode::float64);
        B.Translate(RegisterMode::float64);
        C->mode = RegisterMode::float64;

        C->value.float64 = fmod(A.value.float64, B.value.float64);
        C->Translate(goal);
      }else{
        C->write( A.read() % B.read() );
      }
      break;
    case MathOpperation::exponent:
      A.Translate(RegisterMode::float64);
      B.Translate(RegisterMode::float64);
      C->mode = RegisterMode::float64;

      C->value.float64 = pow(A.value.float64, B.value.float64);
      C->Translate(goal);
      break;
  }
}
void Instance::CmdCopy     (Action *act){
  Register *A = &this->handle[ act->param[0] ];
  Register *B = &this->handle[ act->param[1] ];

  B->mode = A->mode;
  B->write( A->read() );
}