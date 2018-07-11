#pragma once

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
#elif __linux__
  #include "./os/unix.cpp"
#elif __Apple__
  #include "./os/unix.cpp"
  #error Apple devices have untested behaviour
#else
  #error Unhandled OS type
#endif

#include "./sys.cpp"
