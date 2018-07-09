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
      case Commands::compare:
        this->CmdComp(act);
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
      case Commands::move:
        this->CmdMove(act);
        break;

      case Commands::standardStream:
        this->CmdSS(act);
        break;
      case Commands::memory:
        this->CmdMem(act);
        break;

      case Commands::IF:
        if (this->handle[ act->param[0] ].value.uint8 != 1){
          cursor = act->param[1];
        }
        break;

      case Commands::blank:
        break;

      case Commands::GOTO:
        cursor = act->param[0];
        break;

      case Commands::stop:
          return;
        break;
      case Commands::invalid:
        std::cerr << "Warn: Unexpected invalid command" << std::endl;
        std::cerr << "  line: " << act->line << std::endl;
        break;
      case Commands::longCompare:
        this->CmdLComp(act);
        break;
      case Commands::bitwise:
        this->CmdBit(act);
        break;
      case Commands::Loop:
        break;
      default:
        std::cerr << "Warn: Unknown command " << act->command << std::endl;
        std::cerr << "  line: " << act->line << std::endl;
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

  if (this->handle[ act->param[1] ].mode != RegisterMode::uint64){
    std::cerr << "Warn: Attempting to use standard stream with an address register of non-uint64 mode" << std::endl;
    std::cerr << "  Mode: "<<this->handle[ act->param[1] ].mode                                        << std::endl;
    std::cerr << "  Line: "<<act->line                                                                 << std::endl;
  }

  if (act->param[0] == 0){
    if (this->handle[ act->param[2] ].mode != RegisterMode::uint64){
      std::cerr << "Warn: Standard Stream length result register should be in uint64 mode to prevent overloading" << std::endl;
      std::cerr << "  Mode: " << this->handle[ act->param[2] ].mode                                               << std::endl;
      std::cerr << "  Line: " << act->line                                                                        <<std::endl;
    }
    
    std::cin >> str;
    count = sizeof(str);

    // Create a safe space for the input stream data
    //   (Won't get destroyed after function call)
    // Store the information about the space in the 
    // two specified registers
    this->handle[ act->param[1] ].value.address = Memory::Allocate ( count );
    this->handle[ act->param[2] ].write(count);

    // Move the data to the space space
    Memory::Duplicate(this->handle[ act->param[1] ].value.address, &str, count );

  }else{
    count = this->handle[ act->param[2] ].read();
    char *ptr = static_cast<char* >(this->handle[ act->param[1] ].value.address);

    bytes.resize(count);

    for (unsigned long i=0; i<count; i++){
      bytes[i] = *(ptr + i);
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
    this->handle[ act->param[1] ].value.address = Memory::Allocate ( this->handle[ act->param[2] ].read() );
  }else{
    Memory::UnAllocate ( this->handle[ act->param[1] ].value.address );
  }
};
void Instance::CmdPush      (Action *act){
  Handle *director;
  director = static_cast<Handle*>(this->handle[ act->param[1] ].value.address);

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
  director = static_cast<Handle*>(this->handle[ act->param[1] ].value.address);

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
};
void Instance::CmdCopy      (Action *act){
  Register *A = &this->handle[ act->param[0] ];
  Register *B = &this->handle[ act->param[1] ];

  B->mode = A->mode;
  B->write( A->read() );
};
void Instance::CmdMove      (Action *act){
  if (this->handle[ act->param[0] ].mode != RegisterMode::uint64){
    std::cerr << "Warn: Attempting to move data using a register for the from address not in uint64 mode" << std::endl;
    std::cerr << "  Mode: "<<this->handle[ act->param[0] ].mode                                           << std::endl;
    std::cerr << "  Line: "<<act->line                                                                    << std::endl;
  };
  if (this->handle[ act->param[1] ].mode != RegisterMode::uint64){
    std::cerr << "Warn: Attempting to move data using a register for the to address not in uint64 mode" << std::endl;
    std::cerr << "  Mode: "<<this->handle[ act->param[1] ].mode                                         << std::endl;
    std::cerr << "  Line: "<<act->line                                                                  << std::endl;
  };

  Memory::Duplicate(
    this->handle[ act->param[1] ].value.address,  // To
    this->handle[ act->param[0] ].value.address,  // From
    this->handle[ act->param[2] ].read()          // Bytes
  );
}
void Instance::CmdComp      (Action *act){
  Register *A = &this->handle[ act->param[0] ];
  Register *B = &this->handle[ act->param[2] ];
  Register *C = &this->handle[ act->param[3] ];

  switch (A->mode){
    case uint8:
      switch ( B->mode ){
        case uint8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint8 == B->value.uint8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint8 > B->value.uint8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint8 < B->value.uint8 ? true : false;
              break;
          }

          return;
        case int8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint8 == B->value.int8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint8 > B->value.int8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint8 < B->value.int8 ? true : false;
              break;
          }

          return;
        case uint16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint8 == B->value.uint16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint8 > B->value.uint16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint8 < B->value.uint16 ? true : false;
              break;
          }

          return;
        case int16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint8 == B->value.int16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint8 > B->value.int16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint8 < B->value.int16 ? true : false;
              break;
          }

          return;
        case float32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint8 == B->value.float32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint8 > B->value.float32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint8 < B->value.float32 ? true : false;
              break;
          }

          return;
        case uint32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint8 == B->value.uint32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint8 > B->value.uint32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint8 < B->value.uint32 ? true : false;
              break;
          }

          return;
        case int32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint8 == B->value.int32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint8 > B->value.int32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint8 < B->value.int32 ? true : false;
          }

          return;
        case float64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint8 == B->value.float64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint8 > B->value.float64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint8 < B->value.float64 ? true : false;
          }

          return;
        case uint64:

          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint8 == B->value.uint64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint8 > B->value.uint64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint8 < B->value.uint64 ? true : false;
              break;
          }

          return;
        case int64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint8 == B->value.int64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint8 > B->value.int64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint8 < B->value.int64 ? true : false;
              break;
          }

          return;
      }

      break;
    case int8:
      switch ( B->mode ){
        case uint8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int8 == B->value.uint8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int8 > B->value.uint8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int8 < B->value.uint8 ? true : false;
              break;
          }

          return;
        case int8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int8 == B->value.int8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int8 > B->value.int8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int8 < B->value.int8 ? true : false;
              break;
          }

          return;
        case uint16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int8 == B->value.uint16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int8 > B->value.uint16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int8 < B->value.uint16 ? true : false;
              break;
          }

          return;
        case int16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int8 == B->value.int16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int8 > B->value.int16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int8 < B->value.int16 ? true : false;
              break;
          }

          return;
        case float32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int8 == B->value.float32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int8 > B->value.float32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int8 < B->value.float32 ? true : false;
              break;
          }

          return;
        case uint32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int8 == B->value.uint32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int8 > B->value.uint32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int8 < B->value.uint32 ? true : false;
              break;
          }

          return;
        case int32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int8 == B->value.int32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int8 > B->value.int32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int8 < B->value.int32 ? true : false;
              break;
          }

          return;
        case float64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int8 == B->value.float64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int8 > B->value.float64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int8 < B->value.float64 ? true : false;
              break;
          }

          return;
        case uint64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int8 == B->value.uint64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int8 > B->value.uint64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int8 < B->value.uint64 ? true : false;
              break;
          }

          return;
        case int64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int8 == B->value.int64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int8 > B->value.int64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int8 < B->value.int64 ? true : false;
              break;
          }

          return;
      }

      break;
    case uint16:
      switch ( B->mode ){
        case uint8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint16 == B->value.uint8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint16 > B->value.uint8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint16 < B->value.uint8 ? true : false;
              break;
          }

          return;
        case int8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint16 == B->value.int8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint16 > B->value.int8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint16 < B->value.int8 ? true : false;
              break;
          }

          return;
        case uint16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint16 == B->value.uint16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint16 > B->value.uint16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint16 < B->value.uint16 ? true : false;
              break;
          }

          return;
        case int16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint16 == B->value.int16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint16 > B->value.int16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint16 < B->value.int16 ? true : false;
          }

          return;
        case float32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint16 == B->value.float32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint16 > B->value.float32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint16 < B->value.float32 ? true : false;
              break;
          }

          return;
        case uint32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint16 == B->value.uint32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint16 > B->value.uint32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint16 < B->value.uint32 ? true : false;
              break;
          }

          return;
        case int32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint16 == B->value.int32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint16 > B->value.int32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint16 < B->value.int32 ? true : false;
              break;
          }

          return;
        case float64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint16 == B->value.float64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint16 > B->value.float64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint16 < B->value.float64 ? true : false;
              break;
          }

          return;
        case uint64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint16 == B->value.uint64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint16 > B->value.uint64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint16 < B->value.uint64 ? true : false;
              break;
          }

          return;
        case int64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint16 == B->value.int64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint16 > B->value.int64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint16 < B->value.int64 ? true : false;
              break;
          }

          return;
      }

      break;
    case int16:
      switch ( B->mode ){
        case uint8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int16 == B->value.uint8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int16 > B->value.uint8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int16 < B->value.uint8 ? true : false;
              break;
          }

          return;
        case int8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int16 == B->value.int8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int16 > B->value.int8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int16 < B->value.int8 ? true : false;
              break;
          }

          return;
        case uint16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int16 == B->value.uint16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int16 > B->value.uint16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int16 < B->value.uint16 ? true : false;
              break;
          }

          return;
        case int16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int16 == B->value.int16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int16 > B->value.int16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int16 < B->value.int16 ? true : false;
              break;
          }

          return;
        case float32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int16 == B->value.float32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int16 > B->value.float32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int16 < B->value.float32 ? true : false;
              break;
          }

          return;
        case uint32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int16 == B->value.uint32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int16 > B->value.uint32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int16 < B->value.uint32 ? true : false;
              break;
          }

          return;
        case int32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int16 == B->value.int32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int16 > B->value.int32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int16 < B->value.int32 ? true : false;
              break;
          }

          return;
        case float64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int16 == B->value.float64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int16 > B->value.float64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int16 < B->value.float64 ? true : false;
              break;
          }

          return;
        case uint64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int16 == B->value.uint64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int16 > B->value.uint64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int16 < B->value.uint64 ? true : false;
              break;
          }

          return;
        case int64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int16 == B->value.int64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int16 > B->value.int64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int16 < B->value.int64 ? true : false;
              break;
          }

          return;
      }

      break;
    case float32:
      switch ( B->mode ){
        case uint8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float32 == B->value.uint8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float32 > B->value.uint8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float32 < B->value.uint8 ? true : false;
              break;
          }

          return;
        case int8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float32 == B->value.int8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float32 > B->value.int8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float32 < B->value.int8 ? true : false;
              break;
          }

          return;
        case uint16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float32 == B->value.uint16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float32 > B->value.uint16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float32 < B->value.uint16 ? true : false;
              break;
          }

          return;
        case int16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float32 == B->value.int16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float32 > B->value.int16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float32 < B->value.int16 ? true : false;
              break;
          }

          return;
        case float32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float32 == B->value.float32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float32 > B->value.float32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float32 < B->value.float32 ? true : false;
              break;
          }

          return;
        case uint32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float32 == B->value.uint32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float32 > B->value.uint32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float32 < B->value.uint32 ? true : false;
              break;
          }

          return;
        case int32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float32 == B->value.int32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float32 > B->value.int32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float32 < B->value.int32 ? true : false;
              break;
          }

          return;
        case float64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float32 == B->value.float64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float32 > B->value.float64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float32 < B->value.float64 ? true : false;
              break;
          }

          return;
        case uint64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float32 == B->value.uint64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float32 > B->value.uint64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float32 < B->value.uint64 ? true : false;
              break;
          }

          return;
        case int64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float32 == B->value.int64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float32 > B->value.int64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float32 < B->value.int64 ? true : false;
              break;
          }

          return;
      }

      break;
    case uint32:
      switch ( B->mode ){
        case uint8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint32 == B->value.uint8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint32 > B->value.uint8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint32 < B->value.uint8 ? true : false;
              break;
          }

          return;
        case int8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint32 == B->value.int8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint32 > B->value.int8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint32 < B->value.int8 ? true : false;
              break;
          }

          return;
        case uint16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint32 == B->value.uint16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint32 > B->value.uint16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint32 < B->value.uint16 ? true : false;
              break;
          }

          return;
        case int16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint32 == B->value.int16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint32 > B->value.int16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint32 < B->value.int16 ? true : false;
              break;
          }

          return;
        case float32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint32 == B->value.float32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint32 > B->value.float32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint32 < B->value.float32 ? true : false;
              break;
          }

          return;
        case uint32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint32 == B->value.uint32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint32 > B->value.uint32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint32 < B->value.uint32 ? true : false;
              break;
          }

          return;
        case int32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint32 == B->value.int32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint32 > B->value.int32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint32 < B->value.int32 ? true : false;
              break;
          }

          return;
        case float64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint32 == B->value.float64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint32 > B->value.float64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint32 < B->value.float64 ? true : false;
              break;
          }

          return;
        case uint64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint32 == B->value.uint64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint32 > B->value.uint64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint32 < B->value.uint64 ? true : false;
              break;
          }

          return;
        case int64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint32 == B->value.int64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint32 > B->value.int64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint32 < B->value.int64 ? true : false;
              break;
          }

          return;
      }

      break;
    case int32:
      switch ( B->mode ){
        case uint8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int32 == B->value.uint8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int32 > B->value.uint8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int32 < B->value.uint8 ? true : false;
              break;
          }

          return;
        case int8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int32 == B->value.int8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int32 > B->value.int8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int32 < B->value.int8 ? true : false;
              break;
          }

          return;
        case uint16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int32 == B->value.uint16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int32 > B->value.uint16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int32 < B->value.uint16 ? true : false;
              break;
          }

          return;
        case int16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int32 == B->value.int16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int32 > B->value.int16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int32 < B->value.int16 ? true : false;
              break;
          }

          return;
        case float32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int32 == B->value.float32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int32 > B->value.float32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int32 < B->value.float32 ? true : false;
              break;
          }

          return;
        case uint32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int32 == B->value.uint32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int32 > B->value.uint32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int32 < B->value.uint32 ? true : false;
              break;
          }

          return;
        case int32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int32 == B->value.int32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int32 > B->value.int32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int32 < B->value.int32 ? true : false;
              break;
          }

          return;
        case float64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int32 == B->value.float64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int32 > B->value.float64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int32 < B->value.float64 ? true : false;
              break;
          }

          return;
        case uint64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int32 == B->value.uint64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int32 > B->value.uint64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int32 < B->value.uint64 ? true : false;
              break;
          }

          return;
        case int64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int32 == B->value.int64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int32 > B->value.int64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int32 < B->value.int64 ? true : false;
              break;
          }

          return;
      }

      break;
    case float64:
      switch ( B->mode ){
        case uint8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float64 == B->value.uint8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float64 > B->value.uint8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float64 < B->value.uint8 ? true : false;
              break;
          }

          return;
        case int8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float64 == B->value.int8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float64 > B->value.int8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float64 < B->value.int8 ? true : false;
              break;
          }

          return;
        case uint16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float64 == B->value.uint16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float64 > B->value.uint16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float64 < B->value.uint16 ? true : false;
              break;
          }

          return;
        case int16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float64 == B->value.int16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float64 > B->value.int16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float64 < B->value.int16 ? true : false;
              break;
          }

          return;
        case float32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float64 == B->value.float32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float64 > B->value.float32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float64 < B->value.float32 ? true : false;
              break;
          }

          return;
        case uint32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float64 == B->value.uint32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float64 > B->value.uint32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float64 < B->value.uint32 ? true : false;
              break;
          }

          return;
        case int32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float64 == B->value.int32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float64 > B->value.int32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float64 < B->value.int32 ? true : false;
              break;
          }

          return;
        case float64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float64 == B->value.float64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float64 > B->value.float64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float64 < B->value.float64 ? true : false;
              break;
          }

          return;
        case uint64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float64 == B->value.uint64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float64 > B->value.uint64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float64 < B->value.uint64 ? true : false;
              break;
          }

          return;
        case int64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.float64 == B->value.int64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.float64 > B->value.int64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.float64 < B->value.int64 ? true : false;
              break;
          }

          return;
      }

      break;
    case uint64:
      switch ( B->mode ){
        case uint8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint64 == B->value.uint8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint64 > B->value.uint8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint64 < B->value.uint8 ? true : false;
              break;
          }

          return;
        case int8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint64 == B->value.int8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint64 > B->value.int8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint64 < B->value.int8 ? true : false;
              break;
          }

          return;
        case uint16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint64 == B->value.uint16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint64 > B->value.uint16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint64 < B->value.uint16 ? true : false;
              break;
          }

          return;
        case int16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint64 == B->value.int16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint64 > B->value.int16 ? true : false;
              break; 
            case less:
              C->value.uint8 = A->value.uint64 < B->value.int16 ? true : false;
              break;
          }

          return;
        case float32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint64 == B->value.float32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint64 > B->value.float32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint64 < B->value.float32 ? true : false;
              break;
          }

          return;
        case uint32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint64 == B->value.uint32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint64 > B->value.uint32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint64 < B->value.uint32 ? true : false;
              break;
          }

          return;
        case int32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint64 == B->value.int32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint64 > B->value.int32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint64 < B->value.int32 ? true : false;
              break;
          }

          return;
        case float64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint64 == B->value.float64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint64 > B->value.float64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint64 < B->value.float64 ? true : false;
              break;
          }

          return;
        case uint64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint64 == B->value.uint64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint64 > B->value.uint64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint64 < B->value.uint64 ? true : false;
              break;
          }

          return;
        case int64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.uint64 == B->value.int64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.uint64 > B->value.int64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.uint64 < B->value.int64 ? true : false;
              break;
          }

          return;
      }

      break;
    case int64:
      switch ( B->mode ){
        case uint8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int64 == B->value.uint8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int64 > B->value.uint8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int64 < B->value.uint8 ? true : false;
              break;
          }

          return;
        case int8:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int64 == B->value.int8 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int64 > B->value.int8 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int64 < B->value.int8 ? true : false;
              break;
          }

          return;
        case uint16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int64 == B->value.uint16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int64 > B->value.uint16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int64 < B->value.uint16 ? true : false;
              break;
          }

          return;
        case int16:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int64 == B->value.int16 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int64 > B->value.int16 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int64 < B->value.int16 ? true : false;
              break;
          }

          return;
        case float32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int64 == B->value.float32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int64 > B->value.float32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int64 < B->value.float32 ? true : false;
              break;
          }

          return;
        case uint32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int64 == B->value.uint32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int64 > B->value.uint32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int64 < B->value.uint32 ? true : false;
              break;
          }

          return;
        case int32:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int64 == B->value.int32 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int64 > B->value.int32 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int64 < B->value.int32 ? true : false;
              break;
          }

          return;
        case float64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int64 == B->value.float64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int64 > B->value.float64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int64 < B->value.float64 ? true : false;
              break;
          }

          return;
        case uint64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int64 == B->value.uint64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int64 > B->value.uint64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int64 < B->value.uint64 ? true : false;
              break;
          }

          return;
        case int64:
          switch (static_cast<Comparason>(act->param[1])){
            case equal:
              C->value.uint8 = A->value.int64 == B->value.int64 ? true : false;
              break;
            case greater:
              C->value.uint8 = A->value.int64 > B->value.int64 ? true : false;
              break;
            case less:
              C->value.uint8 = A->value.int64 < B->value.int64 ? true : false;
              break;
          }

          return;
      }

      break;
  }

  std::cerr << "Error: Unexpected error, unhandled type comparason case" << std::endl;
  std::cerr << "  A: " << A->mode << std::endl;
  std::cerr << "  B: " << B->mode << std::endl;
  std::cerr << "  C: " << C->mode << std::endl;
  std::cerr << "  line: " << act->line;
}
void Instance::CmdBit       (Action *act){
  unsigned long long A;
  unsigned long long B;

  A = this->handle[ act->param[0] ].value.uint64;
  
  if (act->param[2] == 1){
    B = this->handle[ act->param[3] ].value.uint64;
  }else{
    B = act->param[3];
  }


  switch( static_cast<BitOperator>( act->param[1] ) ){
    case AND:
      this->handle[ act->param[4] ].value.uint64 = A&B;
      break;
    case OR:
      this->handle[ act->param[4] ].value.uint64 = A|B;
      break;
    case XOR:
      this->handle[ act->param[4] ].value.uint64 = A^B;
      break;
    case LeftShift:
      this->handle[ act->param[4] ].value.uint64 = A << B;
      break;
    case RightShift:
      this->handle[ act->param[4] ].value.uint64 = A >> B;
      break;
    case NOT:
      this->handle[ act->param[4] ].value.uint64 = ~A;
      break;
  }
};
void Instance::CmdLComp     (Action *act){
  char *A = static_cast<char *>( this->handle[ act->param[0] ].value.address );
  char *B = static_cast<char *>( this->handle[ act->param[2] ].value.address );

  unsigned long length = act->param[2];

  // Little edian
  if (act->param[4] == 0){
    A += length;
    B += length;

    while (length > 0){
      if (A > B){
        this->handle[ act->param[3] ].value.uint8 = (act->param[1] == Comparason::greater);

        return;
      }else if (A < B){

        this->handle[ act->param[3] ].value.uint8 = (act->param[1] == Comparason::less);
        return;
      }

      length -= 1;
      A--;
      B--;
    }

    this->handle[ act->param[3] ].value.uint8 = (act->param[1] == Comparason::equal);
  }else{
    while (length > 0){
      if (A > B){
        this->handle[ act->param[3] ].value.uint8 = (act->param[1] == Comparason::greater);
        return;
      }else if (A < B){
        this->handle[ act->param[3] ].value.uint8 = (act->param[1] == Comparason::less);
        return;
      }

      length -= 1;
      A++;
      B++;
    }

    this->handle[ act->param[5] ].value.uint8 = (act->param[1] == Comparason::equal);
  }
}