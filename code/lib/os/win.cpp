#include "win.h"

namespace System{
  void SetTextColor(System::Color type){
    switch(type){
      case black:
        system("color 00");
        break;
      case red:
        system("color 04");
        break;
      case green:
        system("color 02");
        break;
      case yellow:
        system("color 06");
        break;
      case blue:
        system("color 01");
        break;
      case megenta:
        system("color 05");
        break;
      case cyan:
        system("color 03");
        break;
      case white:
        system("color 07");
        break;
      
      case brightBlack:
        system("color 08");
        break;
      case brightRed:
        system("color 0C");
        break;
      case brightGreen:
        system("color 0A");
        break;
      case brightYellow:
        system("color 0E");
        break;
      case brightBlue:
        system("color 09");
        break;
      case brightMegenta:
        system("color 0D");
        break;
      case brightCyan:
        system("color 0B");
        break;
      case brightWhite:
        system("color 0F");
        break;
    }
  }
}