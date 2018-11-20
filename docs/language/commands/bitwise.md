# Command: Bit
Applies bitwise opperations similar to the [math](math.md) command.

```fiber
bit {A} {Opperation} {B}
```

## Valid combinations
| A | Opperation | B |
|:-:|:-:|:-:|
| Register Value | ``and``, ``or``, or ``xor`` | Hex |
| • | • | Uint |
| • | • | Int |
| • | • | Register Value |
| • | ``not`` | ``_`` |
| • | shift | Int |

