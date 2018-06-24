#include "register.h"






unsigned long long int Register::read(){
  switch (this->mode){
    case RegisterMode::int8:
    case RegisterMode::uint8:
      return this->value.uint8;

    case RegisterMode::int16:
    case RegisterMode::uint16:
      return this->value.uint16;

    case RegisterMode::int32:
    case RegisterMode::uint32:
      return this->value.uint32;
    case RegisterMode::float32:
      return this->value.float32;

    case RegisterMode::int64:
    case RegisterMode::uint64:
      return this->value.uint64;
    case RegisterMode::float64:
      return this->value.float64;
  }

  std::cerr << "Invalid register mode: " << this->mode;
};
void Register::write(unsigned long long int val){
  switch (this->mode){
    case RegisterMode::int8:
    case RegisterMode::uint8:
      this->value.uint8 = val;

    case RegisterMode::int16:
    case RegisterMode::uint16:
      this->value.uint16 = val;

    case RegisterMode::int32:
    case RegisterMode::uint32:
    case RegisterMode::float32:
      this->value.uint32 = val;

    case RegisterMode::int64:
    case RegisterMode::uint64:
    case RegisterMode::float64:
      this->value.uint64 = val;
  }
};
void Register::Translate(RegisterMode toMode){
  // Don't waste exeution time if nothing will change
  if (this->mode == toMode){
    return;
  }

  switch (this->mode){
    case uint8:
      switch ( toMode ){
        case uint8:
          this->value.uint8 = this->value.uint8;
          this->mode = toMode;
          return;
        case int8:
          this->value.uint8 = this->value.int8;
          this->mode = toMode;
          return;
        case uint16:
          this->value.uint8 = this->value.uint16;
          this->mode = toMode;
          return;
        case int16:
          this->value.uint8 = this->value.int16;
          this->mode = toMode;
          return;
        case float32:
          this->value.uint8 = this->value.float32;
          this->mode = toMode;
          return;
        case uint32:
          this->value.uint8 = this->value.uint32;
          this->mode = toMode;
          return;
        case int32:
          this->value.uint8 = this->value.int32;
          this->mode = toMode;
          return;
        case float64:
          this->value.uint8 = this->value.float64;
          this->mode = toMode;
          return;
        case uint64:
          this->value.uint8 = this->value.uint64;
          this->mode = toMode;
          return;
        case int64:
          this->value.uint8 = this->value.int64;
          this->mode = toMode;
          return;
      }

      break;
    case int8:
      switch ( toMode ){
        case uint8:
          this->value.int8 = this->value.uint8;
          this->mode = toMode;
          return;
        case int8:
          this->value.int8 = this->value.int8;
          this->mode = toMode;
          return;
        case uint16:
          this->value.int8 = this->value.uint16;
          this->mode = toMode;
          return;
        case int16:
          this->value.int8 = this->value.int16;
          this->mode = toMode;
          return;
        case float32:
          this->value.int8 = this->value.float32;
          this->mode = toMode;
          return;
        case uint32:
          this->value.int8 = this->value.uint32;
          this->mode = toMode;
          return;
        case int32:
          this->value.int8 = this->value.int32;
          this->mode = toMode;
          return;
        case float64:
          this->value.int8 = this->value.float64;
          this->mode = toMode;
          return;
        case uint64:
          this->value.int8 = this->value.uint64;
          this->mode = toMode;
          return;
        case int64:
          this->value.int8 = this->value.int64;
          this->mode = toMode;
          return;
      }

      break;
    case uint16:
      switch ( toMode ){
        case uint8:
          this->value.uint16 = this->value.uint8;
          this->mode = toMode;
          return;
        case int8:
          this->value.uint16 = this->value.int8;
          this->mode = toMode;
          return;
        case uint16:
          this->value.uint16 = this->value.uint16;
          this->mode = toMode;
          return;
        case int16:
          this->value.uint16 = this->value.int16;
          this->mode = toMode;
          return;
        case float32:
          this->value.uint16 = this->value.float32;
          this->mode = toMode;
          return;
        case uint32:
          this->value.uint16 = this->value.uint32;
          this->mode = toMode;
          return;
        case int32:
          this->value.uint16 = this->value.int32;
          this->mode = toMode;
          return;
        case float64:
          this->value.uint16 = this->value.float64;
          this->mode = toMode;
          return;
        case uint64:
          this->value.uint16 = this->value.uint64;
          this->mode = toMode;
          return;
        case int64:
          this->value.uint16 = this->value.int64;
          this->mode = toMode;
          return;
      }

      break;
    case int16:
      switch ( toMode ){
        case uint8:
          this->value.int16 = this->value.uint8;
          this->mode = toMode;
          return;
        case int8:
          this->value.int16 = this->value.int8;
          this->mode = toMode;
          return;
        case uint16:
          this->value.int16 = this->value.uint16;
          this->mode = toMode;
          return;
        case int16:
          this->value.int16 = this->value.int16;
          this->mode = toMode;
          return;
        case float32:
          this->value.int16 = this->value.float32;
          this->mode = toMode;
          return;
        case uint32:
          this->value.int16 = this->value.uint32;
          this->mode = toMode;
          return;
        case int32:
          this->value.int16 = this->value.int32;
          this->mode = toMode;
          return;
        case float64:
          this->value.int16 = this->value.float64;
          this->mode = toMode;
          return;
        case uint64:
          this->value.int16 = this->value.uint64;
          this->mode = toMode;
          return;
        case int64:
          this->value.int16 = this->value.int64;
          this->mode = toMode;
          return;
      }

      break;
    case float32:
      switch ( toMode ){
        case uint8:
          this->value.float32 = this->value.uint8;
          this->mode = toMode;
          return;
        case int8:
          this->value.float32 = this->value.int8;
          this->mode = toMode;
          return;
        case uint16:
          this->value.float32 = this->value.uint16;
          this->mode = toMode;
          return;
        case int16:
          this->value.float32 = this->value.int16;
          this->mode = toMode;
          return;
        case float32:
          this->value.float32 = this->value.float32;
          this->mode = toMode;
          return;
        case uint32:
          this->value.float32 = this->value.uint32;
          this->mode = toMode;
          return;
        case int32:
          this->value.float32 = this->value.int32;
          this->mode = toMode;
          return;
        case float64:
          this->value.float32 = this->value.float64;
          this->mode = toMode;
          return;
        case uint64:
          this->value.float32 = this->value.uint64;
          this->mode = toMode;
          return;
        case int64:
          this->value.float32 = this->value.int64;
          this->mode = toMode;
          return;
      }

      break;
    case uint32:
      switch ( toMode ){
        case uint8:
          this->value.uint32 = this->value.uint8;
          this->mode = toMode;
          return;
        case int8:
          this->value.uint32 = this->value.int8;
          this->mode = toMode;
          return;
        case uint16:
          this->value.uint32 = this->value.uint16;
          this->mode = toMode;
          return;
        case int16:
          this->value.uint32 = this->value.int16;
          this->mode = toMode;
          return;
        case float32:
          this->value.uint32 = this->value.float32;
          this->mode = toMode;
          return;
        case uint32:
          this->value.uint32 = this->value.uint32;
          this->mode = toMode;
          return;
        case int32:
          this->value.uint32 = this->value.int32;
          this->mode = toMode;
          return;
        case float64:
          this->value.uint32 = this->value.float64;
          this->mode = toMode;
          return;
        case uint64:
          this->value.uint32 = this->value.uint64;
          this->mode = toMode;
          return;
        case int64:
          this->value.uint32 = this->value.int64;
          this->mode = toMode;
          return;
      }

      break;
    case int32:
      switch ( toMode ){
        case uint8:
          this->value.int32 = this->value.uint8;
          this->mode = toMode;
          return;
        case int8:
          this->value.int32 = this->value.int8;
          this->mode = toMode;
          return;
        case uint16:
          this->value.int32 = this->value.uint16;
          this->mode = toMode;
          return;
        case int16:
          this->value.int32 = this->value.int16;
          this->mode = toMode;
          return;
        case float32:
          this->value.int32 = this->value.float32;
          this->mode = toMode;
          return;
        case uint32:
          this->value.int32 = this->value.uint32;
          this->mode = toMode;
          return;
        case int32:
          this->value.int32 = this->value.int32;
          this->mode = toMode;
          return;
        case float64:
          this->value.int32 = this->value.float64;
          this->mode = toMode;
          return;
        case uint64:
          this->value.int32 = this->value.uint64;
          this->mode = toMode;
          return;
        case int64:
          this->value.int32 = this->value.int64;
          this->mode = toMode;
          return;
      }

      break;
    case float64:
      switch ( toMode ){
        case uint8:
          this->value.float64 = this->value.uint8;
          this->mode = toMode;
          return;
        case int8:
          this->value.float64 = this->value.int8;
          this->mode = toMode;
          return;
        case uint16:
          this->value.float64 = this->value.uint16;
          this->mode = toMode;
          return;
        case int16:
          this->value.float64 = this->value.int16;
          this->mode = toMode;
          return;
        case float32:
          this->value.float64 = this->value.float32;
          this->mode = toMode;
          return;
        case uint32:
          this->value.float64 = this->value.uint32;
          this->mode = toMode;
          return;
        case int32:
          this->value.float64 = this->value.int32;
          this->mode = toMode;
          return;
        case float64:
          this->value.float64 = this->value.float64;
          this->mode = toMode;
          return;
        case uint64:
          this->value.float64 = this->value.uint64;
          this->mode = toMode;
          return;
        case int64:
          this->value.float64 = this->value.int64;
          this->mode = toMode;
          return;
      }

      break;
    case uint64:
      switch ( toMode ){
        case uint8:
          this->value.uint64 = this->value.uint8;
          this->mode = toMode;
          return;
        case int8:
          this->value.uint64 = this->value.int8;
          this->mode = toMode;
          return;
        case uint16:
          this->value.uint64 = this->value.uint16;
          this->mode = toMode;
          return;
        case int16:
          this->value.uint64 = this->value.int16;
          this->mode = toMode;
          return;
        case float32:
          this->value.uint64 = this->value.float32;
          this->mode = toMode;
          return;
        case uint32:
          this->value.uint64 = this->value.uint32;
          this->mode = toMode;
          return;
        case int32:
          this->value.uint64 = this->value.int32;
          this->mode = toMode;
          return;
        case float64:
          this->value.uint64 = this->value.float64;
          this->mode = toMode;
          return;
        case uint64:
          this->value.uint64 = this->value.uint64;
          this->mode = toMode;
          return;
        case int64:
          this->value.uint64 = this->value.int64;
          this->mode = toMode;
          return;
      }

      break;
    case int64:
      switch ( toMode ){
        case uint8:
          this->value.uint64 = this->value.uint8;
          this->mode = toMode;
          return;
        case int8:
          this->value.uint64 = this->value.int8;
          this->mode = toMode;
          return;
        case uint16:
          this->value.uint64 = this->value.uint16;
          this->mode = toMode;
          return;
        case int16:
          this->value.uint64 = this->value.int16;
          this->mode = toMode;
          return;
        case float32:
          this->value.uint64 = this->value.float32;
          this->mode = toMode;
          return;
        case uint32:
          this->value.uint64 = this->value.uint32;
          this->mode = toMode;
          return;
        case int32:
          this->value.uint64 = this->value.int32;
          this->mode = toMode;
          return;
        case float64:
          this->value.uint64 = this->value.float64;
          this->mode = toMode;
          return;
        case uint64:
          this->value.uint64 = this->value.uint64;
          this->mode = toMode;
          return;
        case int64:
          this->value.uint64 = this->value.int64;
          this->mode = toMode;
          return;
      }

      break;
  }

  std::cerr << "Invalid register translation. Cannot translate to: " << toMode << std::endl;
}

int GetRegisterID(std::string str){
  long int result;

  if (str[0] != 'r'){
    return -1;
  }
  
  switch (str[1]){
    case 'A':
      return 0;
    case 'B':
      return 1;
    case 'C':
      return 2;
    case 'D':
      return 3;
    case 'E':
      return 4;
    case 'F':
      return 5;
    case 'G':
      return 6;
    case 'H':
      return 7;
    case 'I':
      return 8;
    case 'J':
      return 9;
    case 'K':
      return 10;
    case 'L':
      return 11;
  }
  
  return -1;
};