# Register
A register is a data handle for a select memory block. A register such as ``rA`` will define the type of data, as well as the address of it (refered to as ``@rA``). To ensure readability for debugging purposes of the intermidate language, when the value of a register is being manipulated you must explicity refer to it with an ``&`` symbole prepending it (e.g ``&rA``).  
There are only nine valid register (``rA``, ``rB``, ..., ``rI``).
