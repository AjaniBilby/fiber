# Internal: Class: Function

Each function only stores byte code relevant to it's own execution path.  
Any sub-functions are not stored within the function it's self, but instead within child functions.  

A function merely describes the executable actions, and has no dynamic bahavior at run time, any execution of a function is handled by an [instance](./instance.md) which will refer to this function to execute.  

## Storage

The code its self is stored within [bytecode](./bytecode.md) which has custom functions to help execution.

Child functions are stored within a vector which is part of the function it's self; this removes chances for reference errors to occur when functions are moved around during inital interpretation.

## Important Terms
**Domain:** The size of the local memory for any instance of this function.
