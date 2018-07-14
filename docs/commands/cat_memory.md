# Memory Management Commands
Commands to move memory within RAM, as well as allocating their positions.

---


## Set
The set command will overwrite the value of a register to a predefined value / or value of another register.

| Attribute Name | Description |
|:--|:--|
|  Register | The register who's value will be changed |
| Value/Register | The value to be set |

---

## Push
Put register A's data at the location specified by register B.
| Attribute Name | Descirption
|:--|:--
| Register A | Data register
| Register B | Address register

---

## Pull
Pull data from register B's addres into register A,
| Attribute Name | Descirption
|:--|:--
| Register A | Data register
| Register B | Address register

---

## Mode
Change the mode of a register. (Will not alter byte values of the register)

| Attribute Name | Descirption
|:--|:--
| Target | Register
| Mode | (u)int[8,16,32,64] or float[32,64]

---

## Translate
Change the mode of a register, while keeping the same information stored (Re-encodes the information in the new type).

| Attribute Name | Descirption
|:--|:--
| Target | Register
| Mode | (u)int[8,16,32,64] or float[32,64]

---

## Math
Applies a mathematic function to two registers storing the result in a third

| Attribute Name | Descirption
|:--|:--
| Value A | Register
| Value B | Register
| Operation | add, subtract, multiply, divide, modulus, exponent
| Result | Register

---

## Move
Duplicates a section of memory to a different location

| Attribute Name | Descirption
|:--|:--
| Pointer A | Register
| Pointer B | Register
| Length | Register

---

## Compare
Compares the neumeric values of two registers

| Attribute Name | Descirption
|:--|:--
| Value A | Register
| Opperator | >, =, <
| Value B | Register
| Result | Register (true=1 | false=0)

---

## lcompare
Compares byte by byte two chunks of memory

| Attribute Name | Descirption
|:--|:--
| Pointer A | Register
| Opperator | >, =, <
| Pointer B | Register
| Length | Register (number of bytes)
| Result | Register (true=1 | false=0)
| Edian | little/big

---

## bit
Does a bit wise operation on two registers

| Attribute Name | Descirption
|:--|:--
| Value A | Register
| Opperator | and/or/xor/lshift/rshift/not
| Value B  | Register / or a static value
| Result | Register

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
