# Command: Memory Allocate
Allows manual dynamic memory allocation.

```fiber
memalloc {type} {address} {amount}
```

## Use
``free`` will free up a chunk of memory defined by the starting address ``address``.  

``alloc`` will allocate a chunk of memory, setting the allocated position to ``address`` with the chunk size being ``amount`` bytes long.


## Valid Combinations
| Type | Address | Amount |
|:-:|:-:|:-:|
| ``alloc`` | Register Address | Register Value |
| ``free`` | • | ``_``|
