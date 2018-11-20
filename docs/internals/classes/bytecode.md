# Internal: Class: Bytecode

The bytecode it's self is a flatted vector of vectors.
If you imagin a single command as being a vector, where the first element defines the command it's self, and the rest define the parameters for that command. The way the bytecode is stored is similar to that, but since commands will never change their size during execution it is flatened.

Each parameter is 8bytes, so we can use a single parameter to define the command and number of parameters it uses as a sort of header. Where each section get's it's own 4bytes of the 8bytes.  
Now during execution the [instance](./instance.md) can jump X elements at a time to move forward between commands since it knows how many parameters each command has.  
Thus there is no explicit data needed to define one element as being the start of a command as uposed to just being a parameter.  

This however produces the problem of traveling backwards, such as to the begining of a loop. This is an unsolveable problem if you are trying to itterate from your current position, as upposed to running through the whole bytecode to find the position one a head of the current.  

