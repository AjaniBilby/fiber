#include "unix.h"

namespace System{
  void SetTextColor(System::Color type){
    char out[8];

    switch(type){
      case black:
        break;
      case red:
        break;
      case green:
        break;
      case yellow:
        break;
      case blue:
        out = { 0x1b, '[', '1', ';', '3', '4', 'm', 0 };
        std::cout << out;
        break;
      case megenta:
        break;
      case cyan:
        break;
      case white:
        break;
      
      case brightBlack:
        break;
      case brightRed:
        break;
      case brightGreen:
        break;
      case brightYellow:
        break;
      case brightBlue:
        break;
      case brightMegenta:
        break;
      case brightCyan:
        break;
      case brightWhite:
        break;
    }
  }
}