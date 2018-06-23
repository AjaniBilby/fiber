## ss

| Attribute Name | Description |
|:--|:--|
| Mode | A static enum value [``in``, ``out``, ``err``, ``log``] |
| Start Register | The register defining the start point of the data to be piped |
| Length Register | The register defining the number of bytes to be piped |

### Mode: Out/Err/Log
The ``Start Register`` and ``Length Register`` are both used in collaboration to specify which bytes to be piped into the standard stream.

### Mode: In
The standard stream input behaves differently, in which the program will hault until data is received, then when data is received  execution will continue, and the value of the two registers supplied will change to point to the received data.
Also registers will be forced into 8byte/64bit mode for the operation.

**NOTE**: these needs to be manually unallocated after use.
