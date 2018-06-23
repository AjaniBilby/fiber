#ifndef System_H
#define System_H

namespace System{
  enum Color: unsigned char{
    black,
    red,
    green,
    yellow,
    blue,
    megenta,
    cyan,
    white,
    brightBlack,
    brightRed,
    brightGreen,
    brightYellow,
    brightBlue,
    brightMegenta,
    brightCyan,
    brightWhite
  };


  void SetTextColor(System::Color type);
};

#ifdef _WIN32
  #include "./os/win.cpp"
#else
  #include "./os/unix.cpp"
#endif


#endif