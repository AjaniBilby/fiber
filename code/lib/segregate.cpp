#include "segregate.h"

namespace Segregate {
  // Splits a single line into individual parameters
  ParamArray Parameterize(std::string str){    
    // Result
    ParamArray res;

    // Working Variables
    bool hadContent = false;
    int toIndex = 0;
    int cursor = 0;
    int vecLen = 0;

    int length = str.length();
    for (int i=0; i<length; i++){
      if (str[i] == ' ' || str[i] == '\t'){
        if (hadContent){
          // Make the vector the correct size to include the new item
          toIndex = vecLen;
          vecLen++;
          res.resize(vecLen);

          // Append new item
          res[toIndex] = str.substr(cursor, i - cursor);
        }

        // Update for next interval
        cursor = i+1;
        hadContent = false; // FALSE
        
        continue;
      }

      hadContent = true;
    }

    // Make sure the last param is not cut off
    if (hadContent){
      // Make the vector the correct size to include the new item
      toIndex = vecLen;
      vecLen++;
      res.resize(vecLen);

      // Append new item
      res[toIndex] = str.substr(cursor);
    }

    return res;
  };

  StrCommands Fragment(std::string str){
    // Result 
    StrCommands res;

    // Working Variables
    int toIndex = 0;
    int cursor = 0;
    int vecLen = 0;

    int length = str.length();
    for (int i=0; i<length; i++){
      // On new line;
      if ( str[i] == '\n'){
        // Make space in the vector for the new item
        toIndex = vecLen;
        vecLen++;
        res.resize(vecLen);

        // Append new item
        if (str[i-1] == '\r'){ // Cut of carrage returns (Windows)
          res[toIndex] = Parameterize( str.substr(cursor, i-cursor-1) );
        }else{
          res[toIndex] = Parameterize( str.substr(cursor, i-cursor) );
        }

        cursor = i+1;
        continue;
      }
    }

    // Ensure the last line isn't cut off
    toIndex = vecLen;
    vecLen++;
    res.resize(vecLen);

    // Append new item
    res[toIndex] = Parameterize( str.substr(cursor) );

    return res;
  };
}