# Memory Management Commands
Commands to move memory within RAM, as well as allocating their positions.

---


## Set
The set command will overwrite the value of a register to a predefined value / or value of another register.

| Attribute Name | Description |
|:--|:--|
|  Register | The register who's value will be changed |
| Value | A static value stated in hexidecimal form or a register |

---

## Mem
Un/Allocate memory

| Attribute Name | Descirption
|:--|:--
| Method | Allocating or unallocating [``alloc``, ``unalloc``] 
| Address Register | The register holding the block address
| Length Register | The register holding the block length

### Mode: Alloc
When allocating the ``Address Register`` value will be overwritten with the resulting address.  
Also the ``Length Register`` must hold the correct value for the size of the block required.

### Mode: Unalloc
When unallocating the ``Address Register`` will specify the block (by location) of be unallocated, and the ``Length Register`` is optional.